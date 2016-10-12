#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

SoftwareSerial midiSerial(2, 3);
MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, midiLooper);

// Configuration
struct configuration {
  byte loopCC[8];
  byte output[128];
};
typedef struct configuration Configuration;
Configuration config;
byte currentOutput;

// Handlers
void handleControlChange(byte channel, byte number, byte value) {
  for (int i = 0 ; i < 8 ; i++) {
    if (config.loopCC[i] == number) {
      // Calcolo la posizione
      byte mask = 1 << i;
      // Controllo il valore
      if (value >= 64) {
        currentOutput = currentOutput | mask;
      } else {
        currentOutput = currentOutput & (~mask);
      }
    }
  }
}

void handleProgramChange(byte channel, byte number) {
  currentOutput = config.output[number];
}

// Main
void initEEPROM() {
  byte loopCCtmp[] = {14, 15, 16, 17, 18, 19, 20, 21};
  memcpy(config.loopCC, loopCCtmp, 8);
  for (int i = 0 ; i < 128 ; i++) {
    config.output[i] = B00000000;
  }
  int eeAddress = 0;
  EEPROM.put(eeAddress, config);
}

void printConfig() {
  Serial.println("Configuration:");
  Serial.print("loopCC: {");
  for (int i = 0 ; i < 8 ; i++) {
    Serial.print(config.loopCC[i]);
    if (i == 7) Serial.println("}");
    else Serial.print(", ");
  };
  for (int i = 0 ; i < 128 ; i++) {
    Serial.print("output[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(config.output[i], BIN);
  };
}

void setup() {
  Serial.begin(9600);
  //initEEPROM();
  int eeAddress = 0;
  EEPROM.get(eeAddress, config);
  //printConfig();
  currentOutput = config.output[0];
  // put your setup code here, to run once:
  midiLooper.setHandleControlChange(handleControlChange);
  midiLooper.setHandleProgramChange(handleProgramChange);
  midiLooper.turnThruOff();
  midiLooper.begin(MIDI_CHANNEL_OMNI);
}

void loop() {
  // put your main code here, to run repeatedly:
  midiLooper.read();
  //printConfig();
}
