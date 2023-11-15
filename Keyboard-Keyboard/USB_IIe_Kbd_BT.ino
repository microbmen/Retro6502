//#include <hidboot.h>
#include <BTHID.h>
#include <usbhub.h>
//#include "KeyboardParser.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>  // Hack to use the SPI library
#endif

USB Usb;
//USBHub Hub(&Usb);
//HIDBoot<HID_PROTOCOL_KEYBOARD> Keyboard(&Usb);
BTD Btd(&Usb);  // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the class in two ways */
// This will start an inquiry and then pair with your device - you only have to do this once
// If you are using a Bluetooth keyboard, then you should type in the password on the keypad and then press enter
//BTHID bthid(&Btd, PAIR, "0000");

// After that you can simply create the instance like so and then press any button on the device
BTHID bthid(&Btd);

//HIDBoot<USB_HID_PROTOCOL_KEYBOARD> HidKeyboard(&Usb);

//Bluetooth Pairing
//Testing make this a 0 (always Pair), normal operation make it 1000. When button pressed it should be 1023.
int PairVal = 000;

//apperently when you first turn on the apple there is some voltage on A4, if we delay a few cycles through the loop it clears up to some very low value.  Hands close by, static, etc. effects this.
int loopcounter = 0;
int PinDelay = 1000;
const int BTPairPin = A4;
/////////////////////


uint32_t next_time;

class KbdRptParser : public KeyboardReportParser {
  void PrintKey(uint8_t mod, uint8_t key);

  void PrintLine(int macro);

protected:
  virtual void OnKeyDown(uint8_t mod, uint8_t key);
  virtual void OnKeyUp(uint8_t mod, uint8_t key);
  virtual void OnKeyPressed(uint8_t key);
};

KbdRptParser Prs;

/*

requirements:
circuits@home code library
https://github.com/felis/USB_Host_Shield_2.0/archive/master.zip


*/

int ENABLE_PIN = 0;
int S0_PIN = 3;
int S1_PIN = 4;
int S2_PIN = 2;
int S3_PIN = 1;

// 4051 pins
int S0_4051 = 5;
int S1_4051 = 6;
int S2_4051 = 7;

int SHIFT_PIN = 14;
int CONTROL_PIN = 15;
int OPEN_APPLE_PIN = 8;
int CLOSED_APPLE_PIN = 16;
int CAPS_LOCK_PIN = 17;
int RESET_PIN = 19;

int r0;
int r1;
int r2;

int c0;
int c1;
int c2;
int c3;

int count = 0;

int CAPS_LOCK_ACTIVE = 0;

const int COLUMNS = 8;
const int ROWS = 10;


// QWERTY
int KEYS_ARRAY[ROWS][COLUMNS] = {
  { 0x29, 0x2B, 0x04, 0x1D, 0x54, 0x27, 0x55, 0x29 },

  { 0x1E, 0x14, 0x07, 0x1B, 0x51, 0x52, 0x50, 0x4F },

  { 0x1F, 0x1a, 0x16, 0x06, 0x62, 0x5C, 0x60, 0x26 },

  { 0x20, 0x08, 0x0B, 0x19, 0x59, 0x5D, 0x61, 0x56 },

  { 0x21, 0x15, 0x09, 0x05, 0x5A, 0x5E, 0x63, 0x58 },

  { 0x23, 0x1C, 0x0A, 0x11, 0x5B, 0x5F, 0x57, 0x85 },

  { 0x22, 0x17, 0x0D, 0x10, 0x31, 0x35, 0x28, 0x2A },

  { 0x24, 0x18, 0x0E, 0x36, 0x2E, 0x13, 0x52, 0x51 },

  { 0x25, 0x0C, 0x33, 0x37, 0x27, 0x2F, 0x2C, 0x50 },

  { 0x26, 0x12, 0x0F, 0x38, 0x2D, 0x30, 0x34, 0x4F }
};

// ASCII to USB HID:
//USB byte, shift key status
const int KEYMAP_SIZE(128);
int KEYMAP[KEYMAP_SIZE][2] = {
  { 0, 0 }, /* NUL */
  { 0, 0 }, /* SOH */
  { 0, 0 }, /* STX */
  { 0, 0 }, /* ETX */
  { 0, 0 }, /* EOT */
  { 0, 0 }, /* ENQ */
  { 0, 0 }, /* ACK */
  { 0, 0 }, /* BEL */
  { 0x2a, 0 },
  /* BS  */ /* Keyboard Delete (Backspace) */
  { 0x2b, 0 },
  /* TAB */ /* Keyboard Tab */
  { 0x28, 0 },
  /* LF  */    /* Keyboard Return (Enter) */
  { 0, 0 },    /* VT  */
  { 0, 0 },    /* FF  */
  { 0, 0 },    /* CR  */
  { 0, 0 },    /* SO  */
  { 0, 0 },    /* SI  */
  { 0, 0 },    /* DEL */
  { 0, 0 },    /* DC1 */
  { 0, 0 },    /* DC2 */
  { 0, 0 },    /* DC3 */
  { 0, 0 },    /* DC4 */
  { 0, 0 },    /* NAK */
  { 0, 0 },    /* SYN */
  { 0, 0 },    /* ETB */
  { 0, 0 },    /* CAN */
  { 0, 0 },    /* EM  */
  { 0, 0 },    /* SUB */
  { 0, 0 },    /* ESC */
  { 0, 0 },    /* FS  */
  { 0, 0 },    /* GS  */
  { 0, 0 },    /* RS  */
  { 0, 0 },    /* US  */
  { 0x2c, 0 }, /*   */
  { 0x1e, 1 }, /* ! */
  { 0x34, 1 }, /* " */
  { 0x20, 1 }, /* # */
  { 0x21, 1 }, /* $ */
  { 0x22, 1 }, /* % */
  { 0x24, 1 }, /* & */
  { 0x34, 0 }, /* ' */
  { 0x26, 1 }, /* ( */
  { 0x27, 1 }, /* ) */
  { 0x25, 1 }, /* * */
  { 0x2e, 1 }, /* + */
  { 0x36, 0 }, /* , */
  { 0x2d, 0 }, /* - */
  { 0x37, 0 }, /* . */
  { 0x38, 0 }, /* / */
  { 0x27, 0 }, /* 0 */
  { 0x1e, 0 }, /* 1 */
  { 0x1f, 0 }, /* 2 */
  { 0x20, 0 }, /* 3 */
  { 0x21, 0 }, /* 4 */
  { 0x22, 0 }, /* 5 */
  { 0x23, 0 }, /* 6 */
  { 0x24, 0 }, /* 7 */
  { 0x25, 0 }, /* 8 */
  { 0x26, 0 }, /* 9 */
  { 0x33, 1 }, /* : */
  { 0x33, 0 }, /* ; */
  { 0x36, 1 }, /* < */
  { 0x2e, 0 }, /* = */
  { 0x37, 1 }, /* > */
  { 0x38, 1 }, /* ? */
  { 0x1f, 1 }, /* @ */
  { 0x04, 1 }, /* A */
  { 0x05, 1 }, /* B */
  { 0x06, 1 }, /* C */
  { 0x07, 1 }, /* D */
  { 0x08, 1 }, /* E */
  { 0x09, 1 }, /* F */
  { 0x0a, 1 }, /* G */
  { 0x0b, 1 }, /* H */
  { 0x0c, 1 }, /* I */
  { 0x0d, 1 }, /* J */
  { 0x0e, 1 }, /* K */
  { 0x0f, 1 }, /* L */
  { 0x10, 1 }, /* M */
  { 0x11, 1 }, /* N */
  { 0x12, 1 }, /* O */
  { 0x13, 1 }, /* P */
  { 0x14, 1 }, /* Q */
  { 0x15, 1 }, /* R */
  { 0x16, 1 }, /* S */
  { 0x17, 1 }, /* T */
  { 0x18, 1 }, /* U */
  { 0x19, 1 }, /* V */
  { 0x1a, 1 }, /* W */
  { 0x1b, 1 }, /* X */
  { 0x1c, 1 }, /* Y */
  { 0x1d, 1 }, /* Z */
  { 0x2f, 0 }, /* [ */
  { 0x31, 0 }, /* \ */
  { 0x30, 0 }, /* ] */
  { 0x23, 1 }, /* ^ */
  { 0x2d, 1 }, /* _ */
  { 0x35, 0 }, /* ` */
  { 0x04, 0 }, /* a */
  { 0x05, 0 }, /* b */
  { 0x06, 0 }, /* c */
  { 0x07, 0 }, /* d */
  { 0x08, 0 }, /* e */
  { 0x09, 0 }, /* f */
  { 0x0a, 0 }, /* g */
  { 0x0b, 0 }, /* h */
  { 0x0c, 0 }, /* i */
  { 0x0d, 0 }, /* j */
  { 0x0e, 0 }, /* k */
  { 0x0f, 0 }, /* l */
  { 0x10, 0 }, /* m */
  { 0x11, 0 }, /* n */
  { 0x12, 0 }, /* o */
  { 0x13, 0 }, /* p */
  { 0x14, 0 }, /* q */
  { 0x15, 0 }, /* r */
  { 0x16, 0 }, /* s */
  { 0x17, 0 }, /* t */
  { 0x18, 0 }, /* u */
  { 0x19, 0 }, /* v */
  { 0x1a, 0 }, /* w */
  { 0x1b, 0 }, /* x */
  { 0x1c, 0 }, /* y */
  { 0x1d, 0 }, /* z */
  { 0x2f, 1 }, /* { */
  { 0x31, 1 }, /* | */
  { 0x30, 1 }, /* } */
  { 0x35, 1 }, /* ~ */
  { 0, 0 }     /* DEL */
};

/*
char* macrostrings[5] = {
  "CALL -151\n",
  "CATALOG\n",
  "1 PRINT \"Hello World\"\n RUN\n",
  "5 rem Joystick Calibration \n10 print pdl(0) \" \" pdl(1) \" \" peek(-16287) \" \" peek(-16286) : goto 10\n    run\n",
  "gr : for i=0 to 39 : color=i-(int(i/16)*16) : vlin 0,39 at i : next \n"
};
// graphics test by Dagen Brock
*/


//printline is pretty much diabled as it takes up too much space.
void KbdRptParser::PrintLine(int macro) {

  int CharDelay = 30;

  //10 print pdl(0) " " pdl(1) " " peek(-16287) " " peek(-16286) : goto 10

  //String MacroString = macrostrings[macro];
  char SHIFTDOWN = 0;

  // break up word into array of characters

  // for each character in array

  // translate character into USB byte

  // send keydown, wait X miliseconds, send keyup

  /*
  for (int letters = 0; letters < MacroString.length(); letters++) {
    //    Serial.print(KEYMAP[MacroString[letters]][0]); // look up the USB byte from the KEYMAP array.

    if (KEYMAP[MacroString[letters]][1] == 1) {
      //     Serial.print("!"); // hold shift
      SHIFTDOWN = 2;
    } else {
      SHIFTDOWN = 0;
    }
    delay(CharDelay);

    OnKeyDown(SHIFTDOWN, KEYMAP[MacroString[letters]][0]);

    delay(CharDelay);
    OnKeyUp(SHIFTDOWN, KEYMAP[MacroString[letters]][0]);
  }
  */
}

void KbdRptParser::PrintKey(uint8_t m, uint8_t key) {
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  /*
    Serial.print((mod.bmLeftCtrl   == 1) ? "C" : " ");
      Serial.print((mod.bmLeftShift  == 1) ? "S" : " ");
      Serial.print((mod.bmLeftAlt    == 1) ? "A" : " ");
      Serial.print((mod.bmLeftGUI    == 1) ? "G" : " ");

      Serial.print(" >");
      PrintHex<uint8_t>(key);
      Serial.print("< ");

      Serial.print((mod.bmRightCtrl   == 1) ? "C" : " ");
      Serial.print((mod.bmRightShift  == 1) ? "S" : " ");
      Serial.print((mod.bmRightAlt    == 1) ? "A" : " ");
      Serial.println((mod.bmRightGUI    == 1) ? "G" : " ");


      */

  // needs to send APPLE keys as key presses, not just as modifiers.

  if (mod.bmLeftAlt == 1) {
    digitalWrite(OPEN_APPLE_PIN, HIGH);
  } else {
    digitalWrite(OPEN_APPLE_PIN, LOW);
  }

  if (mod.bmRightAlt == 1) {
    digitalWrite(CLOSED_APPLE_PIN, HIGH);
  } else {
    digitalWrite(CLOSED_APPLE_PIN, LOW);
  }

  if ((mod.bmLeftShift == 1) || (mod.bmRightShift == 1)) {
    digitalWrite(SHIFT_PIN, LOW);
  } else {
    digitalWrite(SHIFT_PIN, HIGH);
  }

  if ((mod.bmRightCtrl == 1) || (mod.bmLeftCtrl == 1)) {
    digitalWrite(CONTROL_PIN, LOW);
  } else {
    digitalWrite(CONTROL_PIN, HIGH);
  }
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key) {
  //Serial.println("DN ");

  digitalWrite(S0_4051, LOW);
  digitalWrite(S1_4051, LOW);
  digitalWrite(S2_4051, LOW);

  Serial.print(F("Mod: "));
  Serial.print(mod);
  Serial.print(F(" Key: "));
  Serial.println(key);

  PrintKey(mod, key);
  uint8_t c = OemToAscii(mod, key);

  if (c) OnKeyPressed(c);

  int SEARCH_COLUMN = -1;
  int SEARCH_ROW = -1;

  for (int row = 0; row < ROWS; row++) {

    for (int column = 0; column < COLUMNS; column++) {
      if (key == KEYS_ARRAY[row][column]) {
        SEARCH_COLUMN = row;
        SEARCH_ROW = column;
        break;
      }
    }
  }

  //Serial.print("SEARCH_ROW = ");
  //Serial.println(SEARCH_ROW);

  //Serial.print("SEARCH_COLUMN = ");
  //Serial.println(SEARCH_COLUMN);

  //select the column bits
  c0 = bitRead(SEARCH_COLUMN, 0);
  c1 = bitRead(SEARCH_COLUMN, 1);
  c2 = bitRead(SEARCH_COLUMN, 2);
  c3 = bitRead(SEARCH_COLUMN, 3);

  // set the column signals to match selected column
  digitalWrite(S0_PIN, c0);
  digitalWrite(S1_PIN, c1);
  digitalWrite(S2_PIN, c2);
  digitalWrite(S3_PIN, c3);

  //select the row bits
  r0 = bitRead(SEARCH_ROW, 0);
  r1 = bitRead(SEARCH_ROW, 1);
  r2 = bitRead(SEARCH_ROW, 2);

  // set the column signals to match selected column
  digitalWrite(S0_4051, r0);
  digitalWrite(S1_4051, r1);
  digitalWrite(S2_4051, r2);

  // enable the common IO
  digitalWrite(ENABLE_PIN, LOW);

  // defines F12 as "RESET"
  if (key == 69) {  // 0x45 == 69 == key_F12
    digitalWrite(RESET_PIN, LOW);
  }
  // so... control open-apple reset = control alt F12

  // and if that works, then this should do CAPS LOCK:
  if (key == 57) {  // 0x39 == 57 == key_CAPS_LOCK
    // modern caps keys are not locking... usually.

    if (CAPS_LOCK_ACTIVE == 0) {
      digitalWrite(CAPS_LOCK_PIN, LOW);
      CAPS_LOCK_ACTIVE = 1;
    } else {
      digitalWrite(CAPS_LOCK_PIN, HIGH);
      CAPS_LOCK_ACTIVE = 0;
    }
  }

  // trying a sample macro:
  if (key == 58) {  // 0x3A == 58 == key_F1
    PrintLine(0);
  }

  if (key == 59) {  //  == key_F2
    PrintLine(1);
  }

  if (key == 60) {  // == key_F3
    PrintLine(2);
  }

  if (key == 61) {  // == key_F4
    PrintLine(3);
  }

  if (key == 62) {  // == key_F5
    PrintLine(4);
  }
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {

  // disable the common IO
  digitalWrite(ENABLE_PIN, HIGH);

  //    Serial.print("UP ");
  PrintKey(mod, key);


  // defines F12 as "RESET"
  if (key == 69) {  // 0x45 == 69 == key_F12
    digitalWrite(RESET_PIN, HIGH);
  }  //release RESET on KEYUP
  // so... control open-apple reset = control alt F12
}

void KbdRptParser::OnKeyPressed(uint8_t key){
  //Serial.print("ASCII: ");
  //Serial.println((char)key);
};

void BTpair(int ButtonVal, int PairVal) {
  bool ButtonPress = false;
  if (ButtonVal > PairVal) {
    ButtonPress = true;
  }

  if (ButtonPress) {
    //wait 5 seconds due to the Usb.Tasking hanging up for 5 seconds.
    //delay(6000);
    Serial.print(F("Pairing... "));
    Serial.println(ButtonVal);
    bthid.disconnect();
    bthid.pair();
  }
}


void setup() {

  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);
  pinMode(SHIFT_PIN, OUTPUT);
  pinMode(CONTROL_PIN, OUTPUT);
  pinMode(OPEN_APPLE_PIN, OUTPUT);
  pinMode(CLOSED_APPLE_PIN, OUTPUT);
  pinMode(CAPS_LOCK_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(S0_4051, OUTPUT);
  pinMode(S1_4051, OUTPUT);
  pinMode(S2_4051, OUTPUT);

  digitalWrite(ENABLE_PIN, HIGH);  // write low to enable, high to disable
  digitalWrite(S0_PIN, LOW);
  digitalWrite(S1_PIN, LOW);
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);

  digitalWrite(CAPS_LOCK_PIN, HIGH);
  digitalWrite(RESET_PIN, HIGH);

  digitalWrite(OPEN_APPLE_PIN, LOW);
  digitalWrite(CLOSED_APPLE_PIN, LOW);

  // modifier keys - HIGH == not pressed. LOW == pressed.
  digitalWrite(SHIFT_PIN, HIGH);
  digitalWrite(CONTROL_PIN, HIGH);

  digitalWrite(S0_4051, LOW);
  digitalWrite(S1_4051, LOW);
  digitalWrite(S2_4051, LOW);

  // *** DEBUG
  //Serial.begin(115200);
  //Serial.println("Start");

  if (Usb.Init() == -1) {
    Serial.println(F("OSC no start"));
    delay(5000);
    digitalWrite(OPEN_APPLE_PIN, HIGH);
    digitalWrite(CLOSED_APPLE_PIN, HIGH);
    digitalWrite(RESET_PIN, LOW);
    digitalWrite(CONTROL_PIN, LOW);
  }

  delay(200);

  next_time = millis() + 5000;

  while (millis() < 5000) {
    // as soon as the Usb.Tasking happens, pairing stops working, as the keybvoard seems to disconnect, and reconnect a few times. , and if we want to connect without pairing we need to start tasking... so we have a contradiction.
    // The only thing i can think of at the moment is to wait 5 seconds, see if the pairing button is pressed, and if not then start tasking and waiting for an already paired device.
    // Value of the button when pressed should be 1023, but we'll check for anything greater than 1000.
    // If we read it a bunch residual voltage falls off.
    while (loopcounter < PinDelay) {
      int BTPairPinVal = int(analogRead(BTPairPin));
      loopcounter++;
    }
    if (int(analogRead(BTPairPin)) > PairVal) {
      BTpair(int(analogRead(BTPairPin)), PairVal);
      //Serial.print(F("Pairing..."));
      //Serial.println(BTPairPinVal);
      break;
    }
  }

  bthid.SetReportParser(KEYBOARD_PARSER_ID, &Prs);

  // If "Boot Protocol Mode" does not work, then try "Report Protocol Mode"
  // If that does not work either, then uncomment PRINTREPORT in BTHID.cpp to see the raw report
  bthid.setProtocolMode(USB_HID_BOOT_PROTOCOL);  // Boot Protocol Mode
  //bthid.setProtocolMode(HID_RPT_PROTOCOL); // Report Protocol Mode

  //HidKeyboard.SetReportParser(0, (HIDReportParser*)&Prs);
}

void loop() {

  Usb.Task();
}
