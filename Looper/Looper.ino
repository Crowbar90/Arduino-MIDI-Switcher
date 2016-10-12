#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

// Create a software serial port to allow debugging over USB
SoftwareSerial midiSerial(2, 3);
// Create instance of MIDI library, using software serial
MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, midiLooper);

// Configuration
struct configuration {
  // Control Change numbers assigned to each loop
  byte loopCC[8];
  // Output sequences for each Program Change message
  byte output[128];
};
typedef struct configuration Configuration;
Configuration config;
byte currentOutput;

// Handlers
// Control Change: switches a single loop on or off
void handleControlChange(byte channel, byte number, byte value) {
  for (int i = 0 ; i < 8 ; i++) {
    // Individual loops will be switched on and off using bit masks for higher efficiency
    // Check if CC number received matches with one of the loops
    if (config.loopCC[i] == number) {
      // Creates a mask of 0, with a 1 in the position of the loop to switch
      byte mask = 1 << i;
      // Checks value
      if (value >= 64) {
        // Sets a single bit to 1
        currentOutput = currentOutput | mask;
      } else {
        // Sets a single bit to 0
        currentOutput = currentOutput & (~mask);
      }
    }
  }
}
// Program Change: loads a specific output sequence
void handleProgramChange(byte channel, byte number) {
  currentOutput = config.output[number];
}

// Main
// Initialize configuration values
void initEEPROM() {
  // Control Change values for each loop
  byte loopCCtmp[] = {14, 15, 16, 17, 18, 19, 20, 21};
  memcpy(config.loopCC, loopCCtmp, 8);
  // All output configurations are B00000000
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
  printConfig();
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
}
