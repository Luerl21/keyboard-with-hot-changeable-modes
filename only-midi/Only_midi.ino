#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include "MIDIUSB.h"
#include "PitchToNote.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "ArduinoJson.h"


StaticJsonDocument<32> doc;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
uint8_t pressedButtons[3][7] = {};
uint8_t previousButtons[3][7] = {};
const uint8_t cols[7] = {7, 8, 9, 10, 16, 14, 15};
const uint8_t rows[3] = {6, 5, 4};
const uint8_t colskeys = 7;
const uint8_t rowkeys = 3;
const uint8_t allkeys = 21;

bool midi = true;
bool prog = false;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte notePitches[3][7] = {
  pitchC3, pitchD3, pitchE3, pitchF3, pitchG3, pitchA3, pitchB3,
  pitchC4, pitchD4, pitchE4, pitchF4, pitchG4, pitchA4, pitchB4,
  pitchC5, pitchD5, pitchE5, pitchF5, pitchG5, pitchA5, pitchB5,
};

void setup() {
  for (uint8_t i = 0; i < 21; i++) {
    if (i < 7) {
      pinMode(cols[i], INPUT_PULLUP);
    }
    if (i < 3) {
      pinMode(rows[i], OUTPUT);
      digitalWrite(rows[i], HIGH);
    }
  }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  firstFrame();
}

const unsigned short int keys[] = {
  0,  1,  2,  3, 4, 5,  6,
  7,  8, 9, 10, 11, 12, 13,
  14, 15, 16, 17, 18, 19, 20
};

void loop() {
  digitalWrite(rows[0], LOW);
  if (digitalRead(cols[6]) == LOW) {
    if (digitalRead(cols[4]) == LOW && digitalRead(cols[0]) != LOW && digitalRead(cols[1]) != LOW && digitalRead(cols[2]) != LOW && digitalRead(cols[3]) != LOW) {
      prog = true;
      midi = false;
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
    midi = true;
    oledmodes("Midi", true);
  }
  else if (midi) {
    looprowscols();
  }
}
void oledmodes(String x, bool c) {
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

void looprowscols() {
  for (uint8_t i = 0; i < rowkeys; i++) {
    digitalWrite(rows[i], LOW);

    for (uint8_t j = 0; j < colskeys; j++) {
      int keyid = keys[i * colskeys + j];
      if (keyid == -1) {
        continue;
      }
      if (midi) {
        if (pressedButtons[i][j] != previousButtons[i][j]) {
          if (pressedButtons[i][j]) {
            previousButtons[i][j] = 1;
            noteOn(0, notePitches[i][j], 100);
            MidiUSB.flush();
          }
          else {
            previousButtons[i][j] = 0;
            noteOff(0, notePitches[i][j], 0);
            MidiUSB.flush();
          }
        }
        if (digitalRead(cols[j]) == LOW) {
          pressedButtons[i][j] = 1;
          delay(50);
        } else {
          pressedButtons[i][j] = 0;
        }
      }
    }
    digitalWrite(rows[i], HIGH);
  }
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

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
