/*//////////////////////////////////////////////////////////////////////////

microbmen 07/15/2023
mcrobmen@hotmail.com

Version 2

Bluetooth dongle support for RetroConnector
https://github.com/option8/RetroConnector/tree/master/Joystick%20Interface

Works with XBOX BT (1708 v3.x) controllers, and 8BitDO running in XBOX mode (X + Start)

'view' button reverses button 1 and 0

'menu' button turns on rumble for the triggers

3 taps of xbox button disconnects

Turn Trim adjustment fully clockwise (tighten) to place adapter into pairing mode.  Unplug/replug to reboot device.
 - once paired, turn trim to center adjustment and leave there.  Adapter will connect to paired controller each time.

left analog is stretched to reach the corners
right analog is natural values that make an analog circle
d-pad is exact

//////////////////////////////////////////////////////////////////////////*/

//for testing change to 400
const int PairTrim = 100;

#include <avr/pgmspace.h>
#include <Usb.h>
#include <usbhub.h>
#include <usbhid.h>
#include <XBOXONESBT.h>

// Satisfy the IDE, which needs to see the include statement in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

// set pin A0 as the slave select for the digital pot:
const int slaveSelectPin = A0;
const int TrimPin = A3;
int Butt0Pin = A4;
int Butt1Pin = A5;

const int maxX = 153;
int NewmaxX;
const int maxY = 155;
int NewmaxY;

//flags
int FirstRun = 0;
int counterDisconnect = 0;
bool RumbleOn;
bool SnapOn;
bool RStretch;

USB Usb;
BTD Btd(&Usb);  // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the XBOXONESBT class in two ways */
// This will start an inquiry and then pair with the Xbox One S controller - you only have to do this once
// You will need to hold down the Sync and Xbox button at the same time, the Xbox One S controller will then start to blink rapidly indicating that it is in pairing mode
//XBOXONESBT Xbox(&Btd, PAIR);
XBOXONESBT Xbox(&Btd);

void setup() {
  // set the slaveSelectPin as an output:
  pinMode(slaveSelectPin, OUTPUT);

  pinMode(Butt0Pin, OUTPUT);
  pinMode(Butt1Pin, OUTPUT);

  // initialize SPI:
  SPI.begin();

  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial)
    ;  // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1)
      ;  //halt
  }
  float TrimPinVal = float(analogRead(TrimPin));

  Serial.println(F("\r\nRetroConnector XBOX Bluetooth Library Started"));
  Serial.print(F("Calibration set to: "));
  Serial.println(analogRead(TrimPin));

  if (TrimPinVal <= PairTrim) {
    Serial.println(F("Pairing..."));
    Xbox.disconnect();
    Xbox.pair();
  }
}

void loop() {
  Usb.Task();

  //FLAGS
  // We need a flag to use analog vs. digital pads so center doesnt overwrite the other.
  // Digital center will bring all back to center since it is the last buttons looked at.
  int useLeftAnalog = 0;
  int useRightAnalog = 0;

  //Calibrate all initial values to center
  float calibrate = float(analogRead(TrimPin)) / 1023.000;
  float Xcenter = maxX * calibrate;
  float Ycenter = maxY * calibrate;
  //calculate a new max based on center - center *should* be the half way point.
  NewmaxX = (Xcenter * 2);
  NewmaxY = (Ycenter * 2);
  int Xvalue;
  int Yvalue;
  int LXvalue;
  int LYvalue;
  int RXvalue;
  int RYvalue;

  if (Xbox.connected()) {

    // Analogs
    if (Xbox.getAnalogHat(LeftHatX) > 7500 || Xbox.getAnalogHat(LeftHatX) < -7500 || Xbox.getAnalogHat(LeftHatY) > 7500 || Xbox.getAnalogHat(LeftHatY) < -7500) {
      useLeftAnalog = 1;
      if (Xbox.getAnalogHat(LeftHatX) > 7500 || Xbox.getAnalogHat(LeftHatX) < -7500) {
        LXvalue = Xbox.getAnalogHat(LeftHatX);
      } else
        LXvalue = Xcenter;
      if (Xbox.getAnalogHat(LeftHatY) > 7500 || Xbox.getAnalogHat(LeftHatY) < -7500) {
        LYvalue = Xbox.getAnalogHat(LeftHatY);
      } else
        LYvalue = Ycenter;
    }

    //left analog takes the priority
    if ((Xbox.getAnalogHat(RightHatX) > 7500 || Xbox.getAnalogHat(RightHatX) < -7500 || Xbox.getAnalogHat(RightHatY) > 7500 || Xbox.getAnalogHat(RightHatY) < -7500) && useLeftAnalog == 0) {
      useRightAnalog = 1;
      if (Xbox.getAnalogHat(RightHatX) > 7500 || Xbox.getAnalogHat(RightHatX) < -7500) {
        RXvalue = Xbox.getAnalogHat(RightHatX);
      } else
        RXvalue = Xcenter;
      if (Xbox.getAnalogHat(RightHatY) > 7500 || Xbox.getAnalogHat(RightHatY) < -7500) {
        RYvalue = Xbox.getAnalogHat(RightHatY);
      } else
        RYvalue = Ycenter;
    }
    // Write out left analog
    if (useLeftAnalog == 1) {
      // if its not on center, then stretch the value from 0 to twice center with a max of maxX...
      // limit range so we can reach the corners.
      if (LXvalue != (Xcenter)) LXvalue = constrain(map(LXvalue, -22768, 22768, 0, NewmaxX), 0, NewmaxX);
      if (LYvalue != (Ycenter)) LYvalue = constrain(map(LYvalue, -22768, 22768, 0, NewmaxY), 0, NewmaxY);

      Serial.print(F("LAnalog: X: "));
      Serial.print(LXvalue);
      Serial.print(F(" Y: "));
      Serial.print(LYvalue);
      Serial.println();
      if (SnapOn) CornerSnap(LXvalue, LYvalue);
      else sendXYvalues(LXvalue, LYvalue);
    }

    //write out right analog
    if (useRightAnalog == 1) {
      if (RStretch) {
        // stretch
        if (RXvalue != (Xcenter)) RXvalue = constrain(map(RXvalue, -22768, 22768, 0, NewmaxX), 0, NewmaxX);
        if (RYvalue != (Ycenter)) RYvalue = constrain(map(RYvalue, -22768, 22768, 0, NewmaxY), 0, NewmaxY);
      } else {
        // full range
        if (RXvalue != (Xcenter)) RXvalue = constrain(map(RXvalue, -32768, 32768, 0, NewmaxX), 0, NewmaxX);
        if (RYvalue != (Ycenter)) RYvalue = constrain(map(RYvalue, -32768, 32768, 0, NewmaxY), 0, NewmaxY);
      }

      Serial.print(F("RAnalog: X: "));
      Serial.print(RXvalue);
      Serial.print(F(" Y: "));
      Serial.print(RYvalue);
      Serial.println();
      sendXYvalues(RXvalue, RYvalue);
    }

    //D PADs
    if (Xbox.getButtonPress(UP) || Xbox.getButtonPress(DOWN)) {
      if (Xbox.getButtonPress(UP)) {
        Yvalue = 0;
      }
      if (Xbox.getButtonPress(DOWN)) {
        Yvalue = NewmaxY;
      }
    } else
      Yvalue = Ycenter;

    if (Xbox.getButtonPress(LEFT) || Xbox.getButtonPress(RIGHT)) {
      if (Xbox.getButtonPress(LEFT)) {
        Xvalue = 0;
      }
      if (Xbox.getButtonPress(RIGHT)) {
        Xvalue = NewmaxX;
      }
    } else
      Xvalue = Xcenter;

    if (useLeftAnalog == 0 && useRightAnalog == 0) {
      Serial.print(F("Digital: X: "));
      Serial.print(Xvalue);
      Serial.print(F(" Y: "));
      Serial.print(Yvalue);
      Serial.println();
      sendXYvalues(Xvalue, Yvalue);
    }

    // BUTTONS
    if (Xbox.getButtonClick(VIEW)) {
      Serial.println(F("View"));
      //Reverses Buttons
      ButtonReverse();
    }
    if (Xbox.getButtonClick(MENU)) {
      Serial.println(F("Menu"));
      //Toggles Rumble
      RumbleOn = !RumbleOn;
    }
    if (Xbox.getButtonClick(XBOX)) {
      Serial.println(F("Xbox"));
      counterDisconnect++;
      if (counterDisconnect == 3) {
        Xbox.disconnect();
        counterDisconnect = 0;
      }
    }

    if (Xbox.getButtonClick(L3)) {
      Serial.println(F("L3"));
      //Toggles Corner Snap
      SnapOn = !SnapOn;
    }

    if (Xbox.getButtonClick(R3)) {
      Serial.println(F("R3"));
      //Toggles Corner Snap
      RStretch = !RStretch;
    }

    // Set rumble effect if rumble is off (these are for fun and to test)
    if (RumbleOn) Rumble(Xbox.getButtonPress(LT), Xbox.getButtonPress(RT));

    // Button 0 buttons (L3 and R3 dont seem to support press)
    if (Xbox.getButtonPress(B) || Xbox.getButtonPress(X) || Xbox.getButtonPress(RB) || Xbox.getButtonPress(LT)) {
      Serial.println(F("Button 0|1"));
      digitalWrite(Butt0Pin, HIGH);
    } else
      digitalWrite(Butt0Pin, LOW);

    // Button 1 buttons
    if (Xbox.getButtonPress(A) || Xbox.getButtonPress(Y) || Xbox.getButtonPress(LB) || Xbox.getButtonPress(RT)) {
      Serial.println(F("Button 1|0"));
      digitalWrite(Butt1Pin, HIGH);
    } else
      digitalWrite(Butt1Pin, LOW);
  }

  else {
    // get the stick centered even if not connected yet.
    if (FirstRun == 0) {
      FirstRun = 1;
      Serial.print(F("Initial Values: X: "));
      Serial.print(Xcenter);
      Serial.print(F(" Y: "));
      Serial.println(Ycenter);
      sendXYvalues(Xcenter, Ycenter);
    }
  }
}

//Functions
//////////////////////////////////////////

void CornerSnap(int CXvalue, int CYvalue) {
  //corner snap, snaps x and y into the corners when its close
  int pullval = 20;
  //bottom right
  if ((CXvalue >= (NewmaxX - pullval)) && (CYvalue >= (NewmaxY - pullval))) {
    CXvalue = NewmaxX;
    CYvalue = NewmaxY;
  }
  //bottom left
  if ((CXvalue <= pullval) && (CYvalue >= (NewmaxY - pullval))) {
    CXvalue = 0;
    CYvalue = NewmaxY;
  }
  //top right
  if ((CXvalue >= (NewmaxX - pullval)) && (CYvalue < pullval)) {
    CXvalue = NewmaxX;
    CYvalue = 0;
  }
  //top left
  if ((CXvalue <= pullval) && (CYvalue < pullval)) {
    CXvalue = 0;
    CYvalue = 0;
  }
  sendXYvalues(CXvalue, CYvalue);
}

void ButtonReverse() {
  Serial.println(F("Buttons Reversed"));
  //  delay(1);
  if (Butt0Pin == A4) {
    Butt0Pin = A5;
    Butt1Pin = A4;
  } else {
    Butt0Pin = A4;
    Butt1Pin = A5;
  }
}

void Rumble(int FleftRumble, int FrightRumble) {
  static uint16_t oldLTValue, oldRTValue;
  if (FleftRumble != oldLTValue || FrightRumble != oldRTValue) {
    oldLTValue = FleftRumble;
    oldRTValue = FrightRumble;
    uint8_t leftRumble = map(oldLTValue, 0, 1023, 0, 255);  // Map the trigger values into a byte
    uint8_t rightRumble = map(oldRTValue, 0, 1023, 0, 255);
    if (leftRumble > 0 || rightRumble > 0)
      Xbox.setRumbleOn(leftRumble, rightRumble, leftRumble, rightRumble);
    else
      Xbox.setRumbleOff();
  }
}

void sendXYvalues(int Xval, int Yval) {
  int channelX = 5;
  int channelY = 3;

  digitalPotWrite(channelX, Xval);
  digitalPotWrite(channelX - 1, Xval);

  digitalPotWrite(channelY, Yval);
  digitalPotWrite(channelY - 1, Yval);
}

void digitalPotWrite(int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin, HIGH);
}
