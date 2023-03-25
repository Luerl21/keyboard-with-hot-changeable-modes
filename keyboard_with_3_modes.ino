#include <Keyboard.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
//#include "MIDIUSB.h"
//#include "PitchToNote.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "ArduinoJson.h"


StaticJsonDocument<32> doc;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
//uint8_t pressedButtons[3][7] = {};
//uint8_t previousButtons[3][7] = {};
const uint8_t cols[7] = {7, 8, 9, 10, 16, 14, 15};
const uint8_t rows[3] = {6, 5, 4};
const uint8_t colskeys = 7;
const uint8_t rowkeys = 3;
const uint8_t allkeys = 21;

bool phasmophobia = false;
bool midi = false;
bool spad = false;
bool binds = false;
bool prog = false;
bool boolkeyboard = true;

#define KEY_RELEASED   0
#define KEY_PRESSED     1
uint8_t keystate_old[21];
uint8_t keystate[21];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//const byte notePitches[3][7] = {
//  C3, D3, E3, F3, G3, A3, B3,
//  C4, D4, E4, F4, G4, A4, B4,
//  C5, D5, E5, F5, G5, A5, B5,
//};

void setup() {
  for (uint8_t i = 0; i < 21; i++) {
    if (i < 7) {
      pinMode(cols[i], INPUT_PULLUP);
    }
    if (i < 3) {
      pinMode(rows[i], OUTPUT);
      digitalWrite(rows[i], HIGH);
    }
    keystate[i] = KEY_RELEASED;
    keystate_old[i] = KEY_RELEASED;
  }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  firstFrame();
  Keyboard.begin();
}

const unsigned short int keys[] = {
  0,  1,  2,  3, 4, 5,  6,
  7,  8, 9, 10, 11, 12, 13,
  14, 15, 16, 17, 18, 19, 20
};

char characters[] = {
  'q', 'w', 'e', 'r', 't', 'y', 'u',
  'a', 's', 'd', 'f', 'g', 'h', 'j',
  'z', 'x', 'c', 'v', 'b', 'n', 'm'
};

char phasmophobias[] = {
  'g', 'q', 'w', 'e', 'b', 'y', 'u',
  129, 'a', 's', 'd', 'j', 'h', 'j',
  't', 'c', 'f', 'v', 32, 'n', 'm'
};

void loop() {
  digitalWrite(rows[0], LOW);
  if (digitalRead(cols[6]) == LOW) {
//    if (digitalRead(cols[0]) == LOW && digitalRead(cols[1]) != LOW && digitalRead(cols[2]) != LOW && digitalRead(cols[3]) != LOW) {
//      midi = true;
//      spad = false;
//      binds = false;
//      boolkeyboard = false;
//      prog = false;
//      oledmodes("Midi", true);
//    }
    if (digitalRead(cols[2]) == LOW && digitalRead(cols[0]) != LOW && digitalRead(cols[1]) != LOW && digitalRead(cols[3]) != LOW && digitalRead(cols[4]) != LOW) {
      spad = true;
//      midi = false;
      binds = false;
      boolkeyboard = false;
      prog = false;
      oledmodes("SoundPad", true);
    }
    //      else if (digitalRead(cols[2]) == LOW && digitalRead(cols[0]) != LOW && digitalRead(cols[1]) != LOW && digitalRead(cols[3]) != LOW && digitalRead(cols[4]) != LOW) {
    //        binds = true;
    //        midi = false;
    //        spad = false;
    //        boolkeyboard = false;
    //        prog = false;
    //        oledmodes("Binds", true);
    //      }
    else if (digitalRead(cols[1]) == LOW && digitalRead(cols[0]) != LOW && digitalRead(cols[2]) != LOW && digitalRead(cols[3]) != LOW && digitalRead(cols[4]) != LOW) {
      boolkeyboard = true;
      phasmophobia = true;
      binds = false;
//      midi = false;
      spad = false;
      prog = false;
      oledmodes("Phasmophobia", true);
    }
    else if (digitalRead(cols[0]) == LOW && digitalRead(cols[1]) != LOW && digitalRead(cols[2]) != LOW && digitalRead(cols[3]) != LOW && digitalRead(cols[4]) != LOW) {
      boolkeyboard = true;
//      midi = false;
      spad = false;
      binds = false;
      prog = false;
      oledmodes("Keyboard", true);
    }
    else if (digitalRead(cols[4]) == LOW && digitalRead(cols[0]) != LOW && digitalRead(cols[1]) != LOW && digitalRead(cols[2]) != LOW && digitalRead(cols[3]) != LOW) {
      prog = true;
//      midi = false;
      spad = false;
      binds = false;
      boolkeyboard = false;
    }
    digitalWrite(rows[0], HIGH);
  }
  if (prog) {
    Serial.begin(115200);
    for (uint8_t times = 0; times < 3; times++) {
      oledmodes("Prog.", false);
      delay(500);
      oledmodes("Prog..", false);
      delay(500);
      oledmodes("Prog...", false);
      delay(500);
      while (Serial.available() > 0) {
        deserializeJson(doc, Serial);
        eepromwrite(0, doc["firstFrame"]);
      }
    }
    prog = false;
    boolkeyboard = true;
    oledmodes("Keyboard", true);
  }
//  else if (midi) {
//    looprowscols();
//  }
  else if (spad) {
    looprowscols();
    keypress();
    delay(1);
  }
  //  else if (binds && !boolkeyboard && !spad && !midi && !prog) {
  //    looprowscols();
  //    keypress();
  //    delay(1);
  //  }
  else if (boolkeyboard) {
    looprowscols();
    keypress();
    delay(1);
  }
}
void oledmodes(String x, bool c) {
  Keyboard.releaseAll();
  display.clearDisplay();
  display.setTextSize(2);
  oledDisplayCenter(x);
  display.display();
  if (c) {
    delay(3000);
    firstFrame();
  }
  return;
}

void keypress() {
  for (uint8_t i = 0; i < allkeys; i++) {
    if (keystate[i] == KEY_PRESSED && keystate_old[i] == KEY_RELEASED) {
      if (spad || boolkeyboard) {
        if (!boolkeyboard && spad) {
          Keyboard.press(KEY_RIGHT_CTRL);
          Keyboard.press(characters[i]);
        }
        //        if (binds && characters[i] == 'q') {
        //          Keyboard.press(KEY_LEFT_GUI);
        //          Keyboard.press('r');
        //          Keyboard.release(KEY_LEFT_GUI);
        //          Keyboard.release('r');
        //          delay(100);
        //          Keyboard.println("steam://rungameid/1169040");
        //        }
        //        else
        if (boolkeyboard) {
          if (!phasmophobia) {
            Keyboard.press(characters[i]);
          }
          else {
            Keyboard.press(phasmophobias[i]);
          }
        }
      }
    }
    if (keystate[i] == KEY_RELEASED && keystate_old[i] == KEY_PRESSED) {
      if (spad && !boolkeyboard) {
        Keyboard.release(KEY_RIGHT_CTRL);
        Keyboard.release(characters[i]);
      }
      else if (boolkeyboard) {
        if (!phasmophobia) {
          Keyboard.release(characters[i]);
        }
        else {
          Keyboard.release(phasmophobias[i]);
        }
      }
    }
    keystate_old[i] = keystate[i];
  }
}

void looprowscols() {
  for (uint8_t i = 0; i < rowkeys; i++) {
    digitalWrite(rows[i], LOW);

    for (uint8_t j = 0; j < colskeys; j++) {
      int keyid = keys[i * colskeys + j];
      if (keyid == -1) {
        continue;
      }
//      if (midi) {
//        if (pressedButtons[i][j] != previousButtons[i][j]) {
//          if (pressedButtons[i][j]) {
//            previousButtons[i][j] = 1;
//            noteOn(0, notePitches[i][j], 100);
//            MidiUSB.flush();
//          }
//          else {
//            previousButtons[i][j] = 0;
//            noteOff(0, notePitches[i][j], 0);
//            MidiUSB.flush();
//          }
//        }
//        if (digitalRead(cols[j]) == LOW) {
//          pressedButtons[i][j] = 1;
//          delay(50);
//        } else {
//          pressedButtons[i][j] = 0;
//        }
//      }
      else {
        if (digitalRead(cols[j]) == LOW) {
          keystate[keyid] = KEY_PRESSED;
        } else {
          keystate[keyid] = KEY_RELEASED;
        }
      }
    }
    digitalWrite(rows[i], HIGH);
  }
}

//void noteOn(byte channel, byte pitch, byte velocity) {
//  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
//  MidiUSB.sendMIDI(noteOn);
//}
//
//void noteOff(byte channel, byte pitch, byte velocity) {
//  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
//  MidiUSB.sendMIDI(noteOff);
//}

void oledDisplayCenter(String text) {
  uint8_t x1;
  uint8_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, x1, y1, &width, &height);
  display.clearDisplay();
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  display.println(text); // text to display
}
void firstFrame() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.clearDisplay();
  oledDisplayCenter(eepromread(0));
  display.display();
}
void eepromwrite(char add, String data) {
  for (uint8_t i = 0; i < data.length() + 15; i++) {
    EEPROM.update(i, 0);
  }
  for (uint8_t i = 0; i < data.length(); i++) {
    EEPROM.update(add, data[i]);
    add += 1;
  }
}

String eepromread(char add) {
  uint8_t len = 0;
  char data[12];
  unsigned char readValue = EEPROM.read(add);
  while (readValue != '\0') {
    readValue = EEPROM.read(add + len);
    data[len] = readValue;
    len++;
  }
  data[len] = '\0';
  return String(data);
}
//void writeString(char add, String data)
//{
//  uint8_t _size = data.length();
//  uint8_t i;
//  for (i = 0; i < _size; i++)
//  {
//    EEPROM.write(add + i, data[i]);
//  }
//  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
//}
//
//
//String read_String(char add)
//{
//  uint8_t i;
//  char data[16]; //Max 100 Bytes
//  uint8_t len = 0;
//  unsigned char k;
//  k = EEPROM.read(add);
//  while (k != '\0' && len < 500) //Read until null character
//  {
//    k = EEPROM.read(add + len);
//    data[len] = k;
//    len++;
//  }
//  data[len] = '\0';
//  return String(data);
//}
