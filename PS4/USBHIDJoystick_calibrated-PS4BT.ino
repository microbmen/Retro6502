/*//////////////////////////////////////////////////////////////////////////

microbmen 07/27/2023
mcrobmen@hotmail.com

Version 2

Bluetooth dongle support for RetroConnector
https://github.com/option8/RetroConnector/tree/master/Joystick%20Interface

Works with PS4 BT controllers

Successful connection should turn the LED yellow

'View' button reverses button 1 and 0
  turns the LED blue

'Menu' button turns on rumble for the triggers
  turns the LED red

Both Rumble and Reverse turns the LED purple (blue and red :)) 

Tap finger area for a unique apple finger controller that doesnt center
  turns the LED green

3 taps of PS button disconnects

Turn Trim adjustment fully clockwise (tighten) to place adapter into pairing mode.  Unplug/replug to reboot device.
 - once paired, turn trim to center adjustment and leave there.  Adapter will connect to paired controller each time.

Left analog is stretched to reach the corners 
Right analog is natural values that make an analog circle (more accurancy than the left analog)
D-Pad is exact

//////////////////////////////////////////////////////////////////////////*/

//for testing change to 400
const int PairTrim = 100;

//#include <avr/pgmspace.h>
//#include <Usb.h>
#include <usbhub.h>
//#include <usbhid.h>
#include <PS4BT.h>

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

//Max values for X and Y, 155 seems to be the correct voltage for the Apple II (pentimeter to gpio)
const float maxX = 153;
int NewmaxX;
const float maxY = 155;
int NewmaxY;

// we want touch to retain values and not jump to center
float OldTXvalue;
float OldTYvalue;
float TXvalue;
float TYvalue;

//flags
int FirstRun = 0;
int reversedButtons = 0;
int counterDisconnect = 0;
int baseColor = 0;  // 0 blue, 1 yellow, 2 green, 3 red
bool printTouch;
bool RumbleOn;
bool SnapOn;
bool RStretch;

USB Usb;
BTD Btd(&Usb);  // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the PS4BT class in two ways */
// This will start an inquiry and then pair with the PS4 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, the PS4 controller will then start to blink rapidly indicating that it is in pairing mode
//PS4BT PS4(&Btd, PAIR);

// After that you can simply create the instance like so and then press the PS button on the device
PS4BT PS4(&Btd);

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
    ;  // Wait for serial port to connect - used on Leonardo, Teensy and other boards with buiL2-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1)
      ;  //halt
  }
  float TrimPinVal = float(analogRead(TrimPin));

  Serial.println(F("\r\nRetroConnector PS4 Bluetooth Library Started"));
  Serial.print("Calibration set to: ");
  Serial.println(TrimPinVal);

  // no center value should ever be below 100.... (famous last words, but for me its solid)
  if (TrimPinVal <= PairTrim) {
    Serial.println("Pairing...");
    PS4.disconnect();
    PS4.pair();
  }
}

void loop() {

  Usb.Task();

  //FLAGS
  // We need a flag to use analog vs. digital pads so center doesnt overwrite the other.
  // Digital center will bring all back to center since it is the last buttons looked at.
  int useLeftAnalog = 0;
  int useRightAnalog = 0;

  //Calibrate all initial values to center, needs to read in the loop for realtime adj.
  float calibrate = float(analogRead(TrimPin)) / 1023.000;
  float Xcenter = maxX * calibrate;
  float Ycenter = maxY * calibrate;
  //calculate a new max based on center - center *should* be the half way point.
  NewmaxX = (Xcenter * 2);
  NewmaxY = (Ycenter * 2);
  float Xvalue;
  float Yvalue;
  float LXvalue;
  float LYvalue;
  float RXvalue;
  float RYvalue;

  if (PS4.connected()) {

    // set the LED color
    SetLED();
    // DIRECTIONALS
    ////////////////////////////////////////

    // Analogs
    if ((PS4.getAnalogHat(LeftHatX) > 137 || PS4.getAnalogHat(LeftHatX) < 117 || PS4.getAnalogHat(LeftHatY) > 137 || PS4.getAnalogHat(LeftHatY) < 117) && !printTouch) {
      useLeftAnalog = 1;
      if (PS4.getAnalogHat(LeftHatX) > 137 || PS4.getAnalogHat(LeftHatX) < 117) {
        LXvalue = PS4.getAnalogHat(LeftHatX);
      } else
        LXvalue = Xcenter;

      if (PS4.getAnalogHat(LeftHatY) > 137 || PS4.getAnalogHat(LeftHatY) < 117) {
        LYvalue = PS4.getAnalogHat(LeftHatY);
      } else
        LYvalue = Ycenter;
    }

    // left analog take the priority
    if ((PS4.getAnalogHat(RightHatX) > 137 || PS4.getAnalogHat(RightHatX) < 117 || PS4.getAnalogHat(RightHatY) > 137 || PS4.getAnalogHat(RightHatY) < 117) && useLeftAnalog == 0 && !printTouch) {
      useRightAnalog = 1;
      if (PS4.getAnalogHat(RightHatX) > 137 || PS4.getAnalogHat(RightHatX) < 117) {
        RXvalue = PS4.getAnalogHat(RightHatX);
      } else
        RXvalue = Xcenter;
      if (PS4.getAnalogHat(RightHatY) > 137 || PS4.getAnalogHat(RightHatY) < 117) {
        RYvalue = PS4.getAnalogHat(RightHatY);
      } else
        RYvalue = Ycenter;
    }

    // Write out left analog
    if (useLeftAnalog == 1) {
      // if its not on center, then stretch the value from 0 to twice center with a max of maxX...
      // limit range so we can reach the corners.
      if (LXvalue != (maxX * calibrate)) LXvalue = constrain(map(LXvalue, 35, 220, 0, NewmaxX), 0, NewmaxX);
      if (LYvalue != (maxY * calibrate)) LYvalue = constrain(map(LYvalue, 35, 220, 0, NewmaxY), 0, NewmaxY);

      Serial.print("LAnalog: X: ");
      Serial.print(LXvalue);
      Serial.print(" Y: ");
      Serial.print(LYvalue);
      Serial.println();
      if (SnapOn) CornerSnap(LXvalue, LYvalue);
      else sendXYvalues(LXvalue, LYvalue);
    }

    //write out right analog
    if (useRightAnalog == 1) {
      if (RStretch) {
        //stretched
        if (RXvalue != (maxX * calibrate)) RXvalue = constrain(map(RXvalue, 35, 220, 0, NewmaxX), 0, NewmaxX);
        if (RYvalue != (maxY * calibrate)) RYvalue = constrain(map(RYvalue, 35, 220, 0, NewmaxY), 0, NewmaxY);
      } else {
        // full range
        if (RXvalue != (maxX * calibrate)) RXvalue = constrain(map(RXvalue, 0, 255, 0, NewmaxX), 0, NewmaxX);
        if (RYvalue != (maxY * calibrate)) RYvalue = constrain(map(RYvalue, 0, 255, 0, NewmaxY), 0, NewmaxY);
      }

      Serial.print("RAnalog: X: ");
      Serial.print(RXvalue);
      Serial.print(" Y: ");
      Serial.print(RYvalue);
      Serial.println();
      sendXYvalues(RXvalue, RYvalue);
    }

    // Touch pad
    if (PS4.getButtonClick(TOUCHPAD)) {
      Serial.print(F("\r\nTouchpad"));
      // turns blue, but if printtouch is true, then it will immediately turn red below.
      printTouch = !printTouch;  //nice way to do a toggle, negate the boolean
    }
    if (printTouch) {
      if (PS4.isTouching(0)) {  // Print the position of the finger if it is touching the touchpad 0 or 1
        TXvalue = PS4.getX(0);
        TYvalue = PS4.getY(0);
      } else {
        TXvalue = OldTXvalue;
        TYvalue = OldTYvalue;
      }
      OldTXvalue = TXvalue;
      OldTYvalue = TYvalue;

      //write out touch
      if (TXvalue != (maxX * calibrate)) TXvalue = constrain(map(TXvalue, 0, 1919, 0, NewmaxX), 0, NewmaxX);
      if (TYvalue != (maxY * calibrate)) TYvalue = constrain(map(TYvalue, 0, 941, 0, NewmaxY), 0, NewmaxY);

      Serial.print("Touch: X: ");
      Serial.print(TXvalue);
      Serial.print(" Y: ");
      Serial.print(TYvalue);
      Serial.println();
      sendXYvalues(TXvalue, TYvalue);
    }
    //}


    //D PADs
    if (PS4.getButtonPress(UP) || PS4.getButtonPress(DOWN)) {
      if (PS4.getButtonPress(UP)) {
        Yvalue = 0;
      }
      if (PS4.getButtonPress(DOWN)) {
        Yvalue = NewmaxY;
      }
    } else
      Yvalue = Ycenter;

    if (PS4.getButtonPress(LEFT) || PS4.getButtonPress(RIGHT)) {
      if (PS4.getButtonPress(LEFT)) {
        Xvalue = 0;
      }
      if (PS4.getButtonPress(RIGHT)) {
        Xvalue = NewmaxX;
      }
    } else
      Xvalue = Xcenter;

    if (useLeftAnalog == 0 && useRightAnalog == 0 && !printTouch) {
      Serial.print("Digital: X: ");
      Serial.print(Xvalue);
      Serial.print(" Y: ");
      Serial.print(Yvalue);
      Serial.println();
      sendXYvalues(Xvalue, Yvalue);
    }

    // BUTTONS
    ////////////////////////////////////////

    if (PS4.getButtonClick(VIEW)) {
      Serial.println(F("View"));
      //Reverses Buttons
      ButtonReverse();
    }
    if (PS4.getButtonClick(MENU)) {
      Serial.println(F("Menu"));
      ToggleRumble();
    }
    if (PS4.getButtonClick(L3)) {
      Serial.println(F("L3"));
      SnapOn = !SnapOn;
    }
    if (PS4.getButtonClick(R3)) {
      Serial.println(F("R3"));
      RStretch = !RStretch;
    }

    if (PS4.getButtonClick(PS)) {
      Serial.print(F("\r\nPS"));
      counterDisconnect++;
      // when you turn on the controller it registers as 1 press
      if (counterDisconnect == 3) {
        Serial.print(F("\r\nDisconnecting"));
        PS4.disconnect();
        counterDisconnect = -1;
      }
    }

    if (RumbleOn) {
      // times 2 for a bit more rumble initially.
      PS4.setRumbleOn((PS4.getAnalogButton(L2) * 2), (PS4.getAnalogButton(R2) * 2));
    }

    if (PS4.getButtonPress(CIRCLE) || PS4.getButtonPress(SQUARE) || PS4.getButtonPress(R1) || PS4.getButtonPress(L2)) {
      // on 8BitDo B is A, and X is Y
      Serial.println(Butt0Pin);
      digitalWrite(Butt0Pin, HIGH);
    } else
      digitalWrite(Butt0Pin, LOW);

    if (PS4.getButtonPress(CROSS) || PS4.getButtonPress(TRIANGLE) || PS4.getButtonPress(L1) || PS4.getButtonPress(R2)) {
      // on 8BitDo A is B, and Y is X
      Serial.println(Butt1Pin);
      digitalWrite(Butt1Pin, HIGH);
    } else
      digitalWrite(Butt1Pin, LOW);
  } else {
    // First Loop Stuff
    // Get the stick centered even if not connected yet.
    if (FirstRun == 0) {
      FirstRun = 1;
      Serial.print("Initial Values: X: ");
      Serial.print(Xcenter);
      Serial.print(" Y: ");
      Serial.println(Ycenter);
      sendXYvalues(Xcenter, Ycenter);

      //Touch is centered only on the first loop, so that we remeber its position
      TXvalue = Xcenter;
      TYvalue = Ycenter;
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

void SetLED() {  // order counts ...
  if (printTouch) PS4.setLed(Green);
  else if (reversedButtons == 0 && !RumbleOn) PS4.setLed(Yellow);
  else if (reversedButtons == 1 && RumbleOn) PS4.setLed(Purple);
  else if (reversedButtons == 1) PS4.setLed(Blue);
  else if (RumbleOn) PS4.setLed(Red);
  else PS4.setLed(Yellow);
}

void ButtonReverse() {
  Serial.println("Buttons Reversed");
  //  delay(1);
  if (Butt0Pin == A4) {
    Butt0Pin = A5;
    Butt1Pin = A4;
    reversedButtons = 1;
  } else {
    Butt0Pin = A4;
    Butt1Pin = A5;
    reversedButtons = 0;
  }
}

void ToggleRumble() {
  RumbleOn = !RumbleOn;
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
