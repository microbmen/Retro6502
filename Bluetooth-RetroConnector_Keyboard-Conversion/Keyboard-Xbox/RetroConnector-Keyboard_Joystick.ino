/*
This connects inline with an Apple IIe keyboard and maps an XBOX joystick values to keys.

Analog Sticks
Left Analog does Movement - Center stops with 'S'

Right Analog does Gun Movement - Since Castle Wolfenstein doesn’t holster from keyboard, there is no way.  Beyond Castle will do (H)olster.

Directionals (not used in Castle Wolfenstein, but for other games)
    Up / Down / Left / Right – Directional keys. 

Buttons
    X - U for Use
    A - Grenade throw
    Y – Open Apple (Button 0)
    B – Closed Apple (Button 1)
	
    L3 – Cycle’s A-B-C-D (for Beyond Castle Wolfenstein when starting game)
    R3 - CTRL-N - New game for starting the game.

    Left Trigger - Space for searching, opening, etc. items.
    Right Trigger - Fires gun

    Left Shoulder - K - for keyboard to start a game.
    Right Shoulder - RETURN for Inventory
  
    MENU - ESC to exit game.
    VIEW - ESC to exit game.

Every time the controller needs to be re-paired to the card. No way to communicate to the card for pairing. 

*/

#include <hidboot.h>
#include <usbhub.h>
#include <XBOXONESBT.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <../../../../hardware/pic32/libraries/SPI/SPI.h>  // Hack to use the SPI library
#include <SPI.h>                                           // Hack to use the SPI library
#endif

USB Usb;
BTD Btd(&Usb);  // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the XBOXONESBT class in two ways */
// This will start an inquiry and then pair with the Xbox One S controller - you only have to do this once
// You will need to hold down the Sync and Xbox button at the same time, the Xbox One S controller will then start to blink rapidly indicating that it is in pairing mode
XBOXONESBT Xbox(&Btd, PAIR);
//XBOXONESBT Xbox(&Btd);

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

//for debug
int count = 0;
//for a-b-c-d
int cyclecount = 1;
// for sending the centering key after analogs
int Lnonrepeat = 0;
int Rnonrepeat = 0;

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

  // initialize SPI:
  SPI.begin();

  //Serial output breaks the keystrokes, so rememeber to comment this out after debug
  //DEBUG ONLY!
  //Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial)
    ;  // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.println("OSC did not start.");
    delay(5000);
    //Goes into System Test
    digitalWrite(OPEN_APPLE_PIN, HIGH);
    digitalWrite(CLOSED_APPLE_PIN, HIGH);
    digitalWrite(RESET_PIN, LOW);
    digitalWrite(CONTROL_PIN, LOW);
  }

  delay(200);

  Serial.println(F("\r\nRetroConnector XBOX Bluetooth Keyboard Started"));
}

void loop() {
  Usb.Task();

  //FLAGS
  // We need a flag to use analog vs. digital pads so center doesnt overwrite the other.
  // Digital center will bring all back to center since it is the last buttons looked at.
  int useLeftAnalog = 0;
  int useRightAnalog = 0;
  int Xvalue;
  int Yvalue;
  int LXvalue;
  int LYvalue;
  int RXvalue;
  int RYvalue;

  if (Xbox.connected()) {
    // Analogs
    // left analog is for movement
    if (Xbox.getAnalogHat(LeftHatX) > 7500 || Xbox.getAnalogHat(LeftHatX) < -7500 || Xbox.getAnalogHat(LeftHatY) > 7500 || Xbox.getAnalogHat(LeftHatY) < -7500) {
      useLeftAnalog = 1;
      Lnonrepeat = 1;
      if (Xbox.getAnalogHat(LeftHatX) > 7500 || Xbox.getAnalogHat(LeftHatX) < -7500) {
        LXvalue = Xbox.getAnalogHat(LeftHatX);
      } else
        LXvalue = 128;
      if (Xbox.getAnalogHat(LeftHatY) > 7500 || Xbox.getAnalogHat(LeftHatY) < -7500) {
        LYvalue = Xbox.getAnalogHat(LeftHatY);
      } else
        LYvalue = 128;
    }

    //right analog is for gun
    if (Xbox.getAnalogHat(RightHatX) > 7500 || Xbox.getAnalogHat(RightHatX) < -7500 || Xbox.getAnalogHat(RightHatY) > 7500 || Xbox.getAnalogHat(RightHatY) < -7500) {
      useRightAnalog = 1;
      Rnonrepeat = 1;
      if (Xbox.getAnalogHat(RightHatX) > 7500 || Xbox.getAnalogHat(RightHatX) < -7500) {
        RXvalue = Xbox.getAnalogHat(RightHatX);
      } else
        RXvalue = 128;
      if (Xbox.getAnalogHat(RightHatY) > 7500 || Xbox.getAnalogHat(RightHatY) < -7500) {
        RYvalue = Xbox.getAnalogHat(RightHatY);
      } else
        RYvalue = 128;
    }


    // Write out left analog
    if (useLeftAnalog == 1) {
      // normal the values 0 - 255
      if (LXvalue != 128) LXvalue = constrain(map(LXvalue, -22768, 22768, 0, 255), 5, 255);
      if (LYvalue != 128) LYvalue = constrain(map(LYvalue, -22768, 22768, 0, 255), 5, 255);

      //find directions and send letters, else S for stop / center
      //255/3 = 85
      int pullval = 60;

      //bottom right
      if ((LXvalue >= (255 - pullval)) && (LYvalue >= (255 - pullval))) {
        writechar(1, "C");
      }
      //bottom center
      else if (((LXvalue >= (pullval)) && (LXvalue <= (255 - pullval))) && (LYvalue >= (255 - pullval))) {
        writechar(1, "X");
      }
      //bottom left
      else if ((LXvalue <= pullval) && (LYvalue >= (255 - pullval))) {
        writechar(1, "Z");
      }
      //right
      else if ((LXvalue >= (255 - pullval)) && (LYvalue <= (255 - pullval)) && (LYvalue >= (pullval))) {
        writechar(1, "D");
      }
      //left
      else if ((LXvalue <= (pullval)) && (LYvalue <= (255 - pullval)) && (LYvalue >= (pullval))) {
        writechar(1, "A");
      }
      //top right
      else if ((LXvalue >= (255 - pullval)) && (LYvalue <= (pullval))) {
        writechar(1, "E");
      }
      //top center
      else if ((LXvalue >= (pullval) && (LXvalue <= (255 - pullval))) && (LYvalue <= (pullval))) {
        writechar(1, "W");
      }
      //top left
      else if ((LXvalue <= pullval) && (LYvalue <= pullval)) {
        writechar(1, "Q");
      }
      //center  ((LXvalue = 128) && (LYvalue = 128))
      else {
        //Stop walking
        delay(5);
        writechar(1, "S");
      }
    }

    //write out right analog
    if (useRightAnalog == 1) {
      if (RXvalue != 128) RXvalue = constrain(map(RXvalue, -32768, 32768, 0, 255), 5, 255);
      if (RYvalue != 128) RYvalue = constrain(map(RYvalue, -32768, 32768, 0, 255), 5, 255);

      //find directions and send letters, else , nothing , unless we can Holster
      //255/3 = 85
      int pullval_gun = 60;

      //bottom right
      if ((RXvalue >= (255 - pullval_gun)) && (RYvalue >= (255 - pullval_gun))) {
        writechar(0, "?");
      }
      //bottom center
      else if (((RXvalue >= (pullval_gun)) && (RXvalue <= (255 - pullval_gun))) && (RYvalue >= (255 - pullval_gun))) {
        writechar(0, ".");
      }
      //bottom left
      else if ((RXvalue <= pullval_gun) && (RYvalue >= (255 - pullval_gun))) {
        writechar(0, ",");
      }
      //right
      else if ((RXvalue >= (255 - pullval_gun)) && (RYvalue <= (255 - pullval_gun)) && (RYvalue >= (pullval_gun))) {
        writechar(0, ";");
      }
      //left
      else if ((RXvalue <= (pullval_gun)) && (RYvalue <= (255 - pullval_gun)) && (RYvalue >= (pullval_gun))) {
        writechar(1, "K");
      }
      //top right
      else if ((RXvalue >= (255 - pullval_gun)) && (RYvalue <= (pullval_gun))) {
        writechar(1, "P");
      }
      //top center
      else if ((RXvalue >= (pullval_gun) && (RXvalue <= (255 - pullval_gun))) && (RYvalue <= (pullval_gun))) {
        writechar(1, "O");
      }
      //top left
      else if ((RXvalue <= pullval_gun) && (RYvalue <= pullval_gun)) {
        writechar(1, "I");
      }
      //center  ((RXvalue = 128) && (RYvalue = 128))
      else {
        delay(5);
        //holster the gun (only works in beyond)
        //writechar(1, "H");
      }
    }

    // give one centering value when not moving ... in case analog doesnt send it.
    if (useLeftAnalog == 0 && Lnonrepeat == 1) {
      //center  ((LXvalue = 128) && (LYvalue = 128))
      delay(30);
      writechar(1, "S");
      Lnonrepeat = 0;
    }

    if (useRightAnalog == 0 && Rnonrepeat == 1) {
      //center  - holster gun
      delay(30);
      //writechar(1, "H");
      Rnonrepeat = 0;
    }

    if (Xbox.getButtonClick(UP)) {
      writechar(5, "UP");
    }
    if (Xbox.getButtonClick(DOWN)) {
      writechar(5, "DOWN");
    }
    if (Xbox.getButtonClick(LEFT)) {
      writechar(5, "LEFT");
    }
    if (Xbox.getButtonClick(RIGHT)) {
      writechar(5, "RIGHT");
    }

    if (Xbox.getButtonClick(VIEW)) {
      Serial.println(F("View"));
      // CTRL (N)ew Game
      writechar(2, "N");
    }
    if (Xbox.getButtonClick(MENU)) {
      Serial.println(F("Menu"));
      // esc to end game and save
      writechar(5, "ESC");
    }

    if (Xbox.getButtonClick(XBOX)) {
      Serial.println(F("Xbox"));
      // esc to end game and save
      writechar(5, "ESC");
      Xbox.disconnect();
    }

    // to start the game ,  left sholder for (K)eyboard
    if (Xbox.getButtonClick(LB)) {
      //Serial.println(F("LB"));
      writechar(1, "K");
    }
    // right sholder , RETURN for inventory
    if (Xbox.getButtonClick(RB)) {
      //Serial.println(F("RB"));
      // RETURN for Inventory
      writechar(5, "RETURN");
    }

    if (Xbox.getButtonClick(LT)) {
      // Space for searching bodys
      writechar(5, "SPACE");
    }
    if (Xbox.getButtonClick(RT)) {
      //shoot gun
      delay(30);
      writechar(1, "L");
    }
    if (Xbox.getButtonClick(L3)) {
      // Cycles A - B - C
      if (cyclecount == 1) {
        writechar(1, "A");
      }
      if (cyclecount == 2) {
        writechar(1, "B");
      }
      if (cyclecount == 3) {
        writechar(1, "C");
      }
      if (cyclecount == 4) {
        writechar(1, "D");
        cyclecount = 0;
      }
      cyclecount++;
    }
    if (Xbox.getButtonClick(R3)) {
      //Holster Gun
      writechar(1, "H");
    }

    if (Xbox.getButtonClick(A)) {
      //Serial.println(F("A"));
      //grenade
      writechar(1, "T");
    }

    if (Xbox.getButtonClick(B))
      //Serial.println(F("B"));
      //Closed apple
      writechar(4, "CLOSED_APPLE_PIN");

    if (Xbox.getButtonClick(X)) {
      //Serial.println(F("X"));
      // USE
      writechar(1, "U");
    }

    if (Xbox.getButtonClick(Y))
      //Serial.println(F("Y"));
      //open apple to holster gun
      writechar(3, "OPEN_APPLE_PIN");
  }
}

//Functions
//////////////////////////////////////////

void writechar(int MODIFY, String Key_Stroke) {
  uint8_t key = KEYMAP[Key_Stroke[0]][0];
  int SEARCH_COLUMN = -1;
  int SEARCH_ROW = -1;

  /*//for debugging
  Serial.print(Key_Stroke);
  if (count > 50) {
    Serial.println("");
    count = 0;
  }
  count++;
  */
  //////////////////

  digitalWrite(S0_4051, LOW);
  digitalWrite(S1_4051, LOW);
  digitalWrite(S2_4051, LOW);

  //we want shift / CAPS
  if (MODIFY == 1) {
    digitalWrite(SHIFT_PIN, LOW);
  }
  //Hold Ctrl
  if (MODIFY == 2) {
    digitalWrite(CONTROL_PIN, LOW);
    MODIFY = 1;
  }
  // open apple  - they are weird, high is pressed, low is not pressed.
  if (MODIFY == 3) {
    digitalWrite(OPEN_APPLE_PIN, HIGH);
    delay(200);
    digitalWrite(OPEN_APPLE_PIN, LOW);
  }
  // closed apple
  if (MODIFY == 4) {
    digitalWrite(CLOSED_APPLE_PIN, HIGH);
    delay(200);
    digitalWrite(CLOSED_APPLE_PIN, LOW);
  }
  if (MODIFY == 5) {
    //RETURN KEY
    if (Key_Stroke == "RETURN") {
      SEARCH_COLUMN = 6;
      SEARCH_ROW = 6;
    }
    //ESC Key
    if (Key_Stroke == "ESC") {
      SEARCH_COLUMN = 0;
      SEARCH_ROW = 0;
    }
    //SPACE Key
    if (Key_Stroke == "SPACE") {
      SEARCH_COLUMN = 8;
      SEARCH_ROW = 6;
    }
    //UP Key
    if (Key_Stroke == "UP") {
      SEARCH_COLUMN = 7;
      SEARCH_ROW = 6;
    }
    //DOWN Key
    if (Key_Stroke == "DOWN") {
      SEARCH_COLUMN = 7;
      SEARCH_ROW = 7;
    }
    //LEFT Key
    if (Key_Stroke == "LEFT") {
      SEARCH_COLUMN = 8;
      SEARCH_ROW = 7;
    }
    //RIGHT Key
    if (Key_Stroke == "RIGHT") {
      SEARCH_COLUMN = 9;
      SEARCH_ROW = 7;
    }
  }

  //Search
  //Value 0 is lowercase, value 1 is uppercase
  if (MODIFY < 2) {
    for (int row = 0; row < ROWS; row++) {

      for (int column = 0; column < COLUMNS; column++) {
        if (key == KEYS_ARRAY[row][column]) {
          SEARCH_COLUMN = row;
          SEARCH_ROW = column;
          break;
        }
      }
    }
  }

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

  //on repeating key strokes how long do we delay - too long and we get a queue build up though.
  delay(30);
  // disable the common IO
  digitalWrite(SHIFT_PIN, HIGH);
  digitalWrite(CONTROL_PIN, HIGH);
  digitalWrite(ENABLE_PIN, HIGH);
}

////////////////////////////////////////////////////
////// REFERENCE
////////////////////////////////////////////////////

/*
        XO      X1      X2      X3  |   X4      X5      X6      X7
------------------------------------+--------------------------------
YO      ESC     TAB     A       Z   |   /       )       *       ESC
                                    |
Y1      1!      Q       D       X   |   DOWN    UP      LEFT    RIGHT
                                    |
Y2      20      W       S       C   |   0       4       8       (
                                    |
Y3      34      E       H       V   |   1       5       9       -
                                    |
Y4      4$      R       F       B   |   2       6       .       RETURN
                                    |
Y5      6"      Y       G       N   |   3       7       +       ,
                                    +----------------------------------
Y6      5%      T       J       M       \|      `~      RETURN  DELETE

Y7      7&      U       K       ,<      +=       P      UP       DOWN

Y8      8*      I       ;:      .>      0)       [{     SPACE   LEFT

Y9      9(      O       L       /?      -_       ]}      '"      RIGHT



IIe  Col/Row
1     Y0
2     Y1
3     +5V
4     Y2
5     SW1/SAPL*
6     Y3
7     SW0/OAPL*
8     Y4
9     CAPLOCK*
10     Y5
11     CNTL*
12     Y8
13     GND
14     X0
15     RESET*
16     X2
17     X7
18     X1
19     X5
20     X3
21     X4
22     Y9
23     Y6
24     SHFT*
25     Y7
26     X6

*  RESET is a switch between CTL (11) and pin 15
  L and R shift keys share pin 24, switched to ground
  CONTROL switched to ground
  CAPLOCK switched to ground
  pins 5 and 7 are grounded via 470Ω resistors
  Apple keys connect pin 3 (5v) to pins 5 & 7

cd74hc4067  == ATMega pins
mux enable = 0
s0 = 3
s1 = 4
s2 = 2
s3 = 1

to J1
0 = y0
1
2
3
4
5
6
7
8
9 = y9

4051 == ATMega pins
enable = GND (always enabled)
s0 = 5
s1 = 6
s2 = 7

to J1
0 = x0
1
2
3
4
5
6
7 = x7

Solid Apple = AVR 9
Open Apple = AVR 8
shift = AVR A0 / 14
CTL = AVR A1 / 15


*/



/*

	0x00	Reserved (no event indicated)
	0x01	Keyboard ErrorRollOver
	0x02	Keyboard POSTFail
	0x03	Keyboard ErrorUndefined
	0x04	Keyboard a and A
	0x05	Keyboard b and B
	0x06	Keyboard c and C
	0x07	Keyboard d and D
	0x08	Keyboard e and E
	0x09	Keyboard f and F
	0x0A	Keyboard g and G
	0x0B	Keyboard h and H
	0x0C	Keyboard i and I
	0x0D	Keyboard j and J
	0x0E	Keyboard k and K
	0x0F	Keyboard l and L
	0x10	Keyboard m and M
	0x11	Keyboard n and N
	0x12	Keyboard o and O
	0x13	Keyboard p and P
	0x14	Keyboard q and Q
	0x15	Keyboard r and R
	0x16	Keyboard s and S
	0x17	Keyboard t and T
	0x18	Keyboard u and U
	0x19	Keyboard v and V
	0x1A	Keyboard w and W
	0x1B	Keyboard x and X
	0x1C	Keyboard y and Y
	0x1D	Keyboard z and Z
	0x1E	Keyboard 1 and !
	0x1F	Keyboard 2 and @
	0x20	Keyboard 3 and #
	0x21	Keyboard 4 and $
	0x22	Keyboard 5 and %
	0x23	Keyboard 6 and ^
	0x24	Keyboard 7 and &
	0x25	Keyboard 8 and *
	0x26	Keyboard 9 and (
	0x27	Keyboard 0 and )
	0x28	Keyboard Return (ENTER)
	0x29	Keyboard ESCAPE
	0x2A	Keyboard DELETE (Backspace)
	0x2B	Keyboard Tab
	0x2C	Keyboard Spacebar
	0x2D	Keyboard - and (underscore)
	0x2E	Keyboard = and +
	0x2F	Keyboard [ and {
	0x30	Keyboard ] and }
	0x31	Keyboard \ and |
	0x32	Keyboard Non-US # and ~
	0x33	Keyboard ; and :
	0x34	Keyboard ' and "
	0x35	Keyboard Grave Accent and Tilde
	0x36	Keyboard, and <
	0x37	Keyboard . and >
	0x38	Keyboard / and ?
	0x39	Keyboard Caps Lock
	0x3A	Keyboard F1
	0x3B	Keyboard F2
	0x3C	Keyboard F3
	0x3D	Keyboard F4
	0x3E	Keyboard F5
	0x3F	Keyboard F6
	0x40	Keyboard F7
	0x41	Keyboard F8
	0x42	Keyboard F9
	0x43	Keyboard F10
	0x44	Keyboard F11
	0x45	Keyboard F12
	0x46	Keyboard PrintScreen
	0x47	Keyboard Scroll Lock
	0x48	Keyboard Pause
	0x49	Keyboard Insert
	0x4A	Keyboard Home
	0x4B	Keyboard PageUp
	0x4C	Keyboard Delete Forward
	0x4D	Keyboard End
	0x4E	Keyboard PageDown
	0x4F	Keyboard RightArrow
	0x50	Keyboard LeftArrow
	0x51	Keyboard DownArrow
	0x52	Keyboard UpArrow
	0x53	Keypad Num Lock and Clear
	0x54	Keypad /
	0x55	Keypad *
	0x56	Keypad -
	0x57	Keypad +
	0x58	Keypad ENTER
	0x59	Keypad 1 and End
	0x5A	Keypad 2 and Down Arrow
	0x5B	Keypad 3 and PageDn
	0x5C	Keypad 4 and Left Arrow
	0x5D	Keypad 5
	0x5E	Keypad 6 and Right Arrow
	0x5F	Keypad 7 and Home
	0x60	Keypad 8 and Up Arrow
	0x61	Keypad 9 and PageUp
	0x62	Keypad 0 and Insert
	0x63	Keypad . and Delete
	0x64	Keyboard Non-US \ and |
	0x65	Keyboard Application
	0x66	Keyboard Power
	0x67	Keypad =
	0x68	Keyboard F13
	0x69	Keyboard F14
	0x6A	Keyboard F15
	0x6B	Keyboard F16
	0x6C	Keyboard F17
	0x6D	Keyboard F18
	0x6E	Keyboard F19
	0x6F	Keyboard F20
	0x70	Keyboard F21
	0x71	Keyboard F22
	0x72	Keyboard F23
	0x73	Keyboard F24
	0x74	Keyboard Execute
	0x75	Keyboard Help
	0x76	Keyboard Menu
	0x77	Keyboard Select
	0x78	Keyboard Stop
	0x79	Keyboard Again
	0x7A	Keyboard Undo
	0x7B	Keyboard Cut
	0x7C	Keyboard Copy
	0x7D	Keyboard Paste
	0x7E	Keyboard Find
	0x7F	Keyboard Mute
	0x80	Keyboard Volume Up
	0x81	Keyboard Volume Down
	0x82	Keyboard Locking Caps Lock
	0x83	Keyboard Locking Num Lock
	0x84	Keyboard Locking Scroll Lock
	0x85	Keypad Comma
	0x86	Keypad Equal Sign
	0x87	Keyboard International1
	0x88	Keyboard International2
	0x89	Keyboard International3
	0x8A	Keyboard International4
	0x8B	Keyboard International5
	0x8C	Keyboard International6
	0x8D	Keyboard International7
	0x8E	Keyboard International8
	0x8F	Keyboard International9
	0x90	Keyboard LANG1
	0x91	Keyboard LANG2
	0x92	Keyboard LANG3
	0x93	Keyboard LANG4
	0x94	Keyboard LANG5
	0x95	Keyboard LANG6
	0x96	Keyboard LANG7
	0x97	Keyboard LANG8
	0x98	Keyboard LANG9
	0x99	Keyboard Alternate Erase
	0x9A	Keyboard SysReq/Attention
	0x9B	Keyboard Cancel
	0x9C	Keyboard Clear
	0x9D	Keyboard Prior
	0x9E	Keyboard Return
	0x9F	Keyboard Separator
	0xA0	Keyboard Out
	0xA1	Keyboard Oper
	0xA2	Keyboard Clear/Again
	0xA3	Keyboard CrSel/Props
	0xA4	Keyboard ExSel
	0xE0	Keyboard LeftControl
	0xE1	Keyboard LeftShift
	0xE2	Keyboard LeftAlt
	0xE3	Keyboard Left GUI
	0xE4	Keyboard RightControl
	0xE5	Keyboard RightShift
	0xE6	Keyboard RightAlt
	0xE7	Keyboard Right GUI


*/
