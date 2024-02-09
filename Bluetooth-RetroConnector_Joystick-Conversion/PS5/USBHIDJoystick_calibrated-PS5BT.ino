/*//////////////////////////////////////////////////////////////////////////

microbmen 07/27/2023
mcrobmen@hotmail.com

Version 1  (PS5 takes up a lot of extra space so this code is much more optimised than PS4)

Bluetooth dongle support for RetroConnector
https://github.com/option8/RetroConnector/tree/master/Joystick%20Interface

Works with PS5 BT controllers

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

#include <usbhub.h>
#include <PS5BT.h>

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
const int maxX = 153;
int NewmaxX;
const int maxY = 155;
int NewmaxY;

// we want touch to retain values and not jump to center
int OldTXvalue;
int OldTYvalue;
int TXvalue;
int TYvalue;

//flags
uint16_t lastMessageCounter = -1;
int FirstRun = 0;
int reverseButs = 0;
int counterDis = 0;
int baseColor = 0;  // 0 blue, 1 yellow, 2 green, 3 red
bool RumbleOn;
bool printTouch;
bool SnapOn;
bool RStretch;

USB Usb;
BTD Btd(&Usb);  // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the PS5BT class in two ways */
// This will start an inquiry and then pair with the PS5 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, the PS5 controller will then start to blink rapidly indicating that it is in pairing mode
//PS5BT PS5(&Btd, PAIR);

// After that you can simply create the instance like so and then press the PS button on the device
PS5BT PS5(&Btd);

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
    Serial.print(F("\r\nno OSC"));
    while (1)
      ;  //halt
  }
  float TrimPinVal = float(analogRead(TrimPin));

  Serial.println(F("\r\nRetroConnector PS5 BT"));
  Serial.print(F("Calibrate: "));
  Serial.println(TrimPinVal);

  // no center value should ever be below 100.... (famous last words, but for me its solid)
  if (TrimPinVal <= PairTrim) {
    Serial.println(F("Pair"));
    PS5.disconnect();
    PS5.pair();
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
  int Xcenter = (maxX * calibrate);
  int Ycenter = (maxY * calibrate);
  NewmaxX = (Xcenter * 2);
  NewmaxY = (Ycenter * 2);
  int LXvalue;
  int LYvalue;
  int RXvalue;
  int RYvalue;
  int Yvalue;
  int Xvalue;

  if (PS5.connected() && lastMessageCounter != PS5.getMessageCounter()) {
    lastMessageCounter = PS5.getMessageCounter();

    // set the LED color
    SetLED();
    // DIRECTIONALS
    ////////////////////////////////////////

    // Analogs
    if ((PS5.getAnalogHat(LeftHatX) > 137 || PS5.getAnalogHat(LeftHatX) < 117 || PS5.getAnalogHat(LeftHatY) > 137 || PS5.getAnalogHat(LeftHatY) < 117) && !printTouch) {
      useLeftAnalog = 1;
      if (PS5.getAnalogHat(LeftHatX) > 137 || PS5.getAnalogHat(LeftHatX) < 117) {
        LXvalue = PS5.getAnalogHat(LeftHatX);
      } else
        LXvalue = Xcenter;

      if (PS5.getAnalogHat(LeftHatY) > 137 || PS5.getAnalogHat(LeftHatY) < 117) {
        LYvalue = PS5.getAnalogHat(LeftHatY);
      } else
        LYvalue = Ycenter;
      // if its not on center, then stretch the value from 0 to twice center with a max of maxX...
      // limit range so we can reach the corners.
      if (LXvalue != (Xcenter)) LXvalue = constrain(map(LXvalue, 35, 220, 0, (NewmaxX)), 0, (NewmaxX));
      if (LYvalue != (Ycenter)) LYvalue = constrain(map(LYvalue, 35, 220, 0, (NewmaxY)), 0, (NewmaxY));

      //Serial.print(F("LA:"));
      if (SnapOn) {
        CornerSnap(LXvalue, LYvalue);
      } else {
        serialSend(LXvalue, LYvalue);
      }
    }

    if ((PS5.getAnalogHat(RightHatX) > 137 || PS5.getAnalogHat(RightHatX) < 117 || PS5.getAnalogHat(RightHatY) > 137 || PS5.getAnalogHat(RightHatY) < 117) && useLeftAnalog == 0 && !printTouch) {
      useRightAnalog = 1;
      if (PS5.getAnalogHat(RightHatX) > 137 || PS5.getAnalogHat(RightHatX) < 117) {
        RXvalue = PS5.getAnalogHat(RightHatX);
      } else
        RXvalue = Xcenter;
      if (PS5.getAnalogHat(RightHatY) > 137 || PS5.getAnalogHat(RightHatY) < 117) {
        RYvalue = PS5.getAnalogHat(RightHatY);
      } else
        RYvalue = Ycenter;

      if (RStretch) {
        if (RXvalue != (Xcenter)) RXvalue = constrain(map(RXvalue, 35, 220, 0, (NewmaxX)), 0, (NewmaxX));
        if (RYvalue != (Ycenter)) RYvalue = constrain(map(RYvalue, 35, 220, 0, (NewmaxY)), 0, (NewmaxY));
      } else {
        if (RXvalue != (Xcenter)) RXvalue = constrain(map(RXvalue, 0, 255, 0, (NewmaxX)), 0, (NewmaxX));
        if (RYvalue != (Ycenter)) RYvalue = constrain(map(RYvalue, 0, 255, 0, (NewmaxY)), 0, (NewmaxY));
      }

      //Serial.print(F("RA:"));
      serialSend(RXvalue, RYvalue);
    }

    // Touch pad
    if (PS5.getButtonClick(TOUCHPAD)) {
      //Serial.println(F("Touch"));
      // turns blue, but if printtouch is true, then it will immediately turn red below.
      printTouch = !printTouch;  //nice way to do a toggle, negate the boolean
    }
    if (printTouch) {
      if (PS5.isTouching(0)) {  // Print the position of the finger if it is touching the touchpad 0 or 1
        TXvalue = PS5.getX(0);
        TYvalue = PS5.getY(0);
      } else {
        TXvalue = OldTXvalue;
        TYvalue = OldTYvalue;
      }
      OldTXvalue = TXvalue;
      OldTYvalue = TYvalue;

      //write out touch
      if (TXvalue != (Xcenter)) TXvalue = constrain(map(TXvalue, 0, 1919, 0, (NewmaxX)), 0, (NewmaxX));
      if (TYvalue != (Ycenter)) TYvalue = constrain(map(TYvalue, 0, 941, 0, (NewmaxY)), 0, (NewmaxY));

      //Serial.print(F("T:"));
      serialSend(TXvalue, TYvalue);
    }
    //}


    //D PADs
    if (PS5.getButtonPress(UP) || PS5.getButtonPress(DOWN)) {
      if (PS5.getButtonPress(UP)) {
        Yvalue = 0;
      }
      if (PS5.getButtonPress(DOWN)) {
        Yvalue = NewmaxY;
      }
    } else
      Yvalue = Ycenter;

    if (PS5.getButtonPress(LEFT) || PS5.getButtonPress(RIGHT)) {
      if (PS5.getButtonPress(LEFT)) {
        Xvalue = 0;
      }
      if (PS5.getButtonPress(RIGHT)) {
        Xvalue = NewmaxX;
      }
    } else
      Xvalue = Xcenter;

    if (useLeftAnalog == 0 && useRightAnalog == 0 && !printTouch) {
      //Serial.print(F("D:"));
      serialSend(Xvalue, Yvalue);
    }

    // BUTTONS
    ////////////////////////////////////////

    if (PS5.getButtonClick(CREATE)) {
      //Serial.println(F("Reverse"));
      //Serial.println(F("Create"));
      //Reverses Buttons
      ButtonReverse();
    }
    if (PS5.getButtonClick(OPTIONS)) {
      //Serial.println(F("Rumble"));
      //Serial.println(F("Options"));
      ToggleRumble();
    }

    if (PS5.getButtonClick(L3)) {
      //Serial.println(F("L3"));
      SnapOn = !SnapOn;
    }

    if (PS5.getButtonClick(R3)) {
      //Serial.println(F("R3"));
      RStretch = !RStretch;
    }

    if (PS5.getButtonClick(PS)) {
      //Serial.println(F("PS"));
      counterDis++;
      // when you turn on the controller it registers as 1 press
      if (counterDis == 3) {
        //Serial.println(F("Disconnect"));
        PS5.disconnect();
        counterDis = -1;
      }
    }

    if (RumbleOn) {
      // times 2 for a bit more rumble initially.
      PS5.setRumbleOn((PS5.getAnalogButton(L2) * 2), (PS5.getAnalogButton(R2) * 2));
    }

    if (PS5.getButtonPress(CIRCLE) || PS5.getButtonPress(SQUARE) || PS5.getButtonPress(R1) || PS5.getButtonPress(L2)) {
      // on 8BitDo B is A, and X is Y
      Serial.println(Butt0Pin);
      digitalWrite(Butt0Pin, HIGH);
    } else
      digitalWrite(Butt0Pin, LOW);

    if (PS5.getButtonPress(CROSS) || PS5.getButtonPress(TRIANGLE) || PS5.getButtonPress(L1) || PS5.getButtonPress(R2)) {
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
      Serial.print(F("Init:"));
      serialSend((Xcenter), (Ycenter));

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
  serialSend(CXvalue, CYvalue);
}

void SetLED() {  // order counts ...
  if (printTouch) PS5.setLed(Green);
  else if (reverseButs == 0 && RumbleOn == 0) PS5.setLed(Yellow);
  else if (reverseButs == 1 && RumbleOn == 1) PS5.setLed(Purple);
  else if (reverseButs == 1) PS5.setLed(Blue);
  else if (RumbleOn) PS5.setLed(Red);
  else PS5.setLed(Yellow);
  // analog special features
  if (SnapOn && !RStretch) PS5.setPlayerLed(1);
  else if (!SnapOn && RStretch) PS5.setPlayerLed(2);
  else if (SnapOn && RStretch) PS5.setPlayerLed(3);
  else PS5.setPlayerLed(0);
}

void ButtonReverse() {
  //Serial.println("Reverse");
  if (Butt0Pin == A4) {
    Butt0Pin = A5;
    Butt1Pin = A4;
    reverseButs = 1;
  } else {
    Butt0Pin = A4;
    Butt1Pin = A5;
    reverseButs = 0;
  }
}

void ToggleRumble() {
  //Toggles Rumble
  RumbleOn = !RumbleOn;
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

//print to serial and send to POTs
void serialSend(int xVal, int yVal) {
  int channelX = 5;
  int channelY = 3;

  //Serial.print("X:");
  //Serial.print(xVal);
  //Serial.print(F(" "));
  //Serial.print(yVal);
  //Serial.println(F(""));

  digitalPotWrite(channelX, xVal);
  digitalPotWrite(channelX - 1, xVal);
  digitalPotWrite(channelY, yVal);
  digitalPotWrite(channelY - 1, yVal);
}
