#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

// Create a software serial port to allow debugging over USB
//SoftwareSerial midiSerial(2, 3); // RX, TX
// Create instance of MIDI library, using software serial
//MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, midiLooper);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiLooper);

// Onboard LED
int ledPin = 13;
//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;

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
  //Serial.println("CC");
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
      updateOutput();
    }
  }
  // Store
  if (number == 22) {
    // Edit config
    config.output[value] = currentOutput;
    // Store into EEPROM
    int eeAddress = 0;
    digitalWrite(ledPin, HIGH);
    EEPROM.put(eeAddress, config);
    digitalWrite(ledPin, LOW);
  }
  //Serial.print("output: ");
  //Serial.println(currentOutput, BIN);
}
// Program Change: loads a specific output sequence
void handleProgramChange(byte channel, byte number) {
  currentOutput = config.output[number];
  updateOutput();
  //Serial.print("output: ");
  //Serial.println(currentOutput, BIN);
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
  digitalWrite(ledPin, HIGH);
  EEPROM.put(eeAddress, config);
  digitalWrite(ledPin, LOW);
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

void updateOutput() {
    // the LEDs don't change while you're sending in bits:
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, MSBFIRST, currentOutput);  
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
}

void setup() {
  // Sets the pins used for the shift register as output
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  
  // LED pin, used to monitor EEPROM activity
  pinMode(ledPin, OUTPUT);
  
  // Uncomment this line if the EEPROM needs to be initialized again
  // initEEPROM();
  
  // Read configuration from EEPROM
  int eeAddress = 0;
  EEPROM.get(eeAddress, config);

  // Sets first output
  currentOutput = config.output[0];
  updateOutput();
  
  // Disable MIDI Thru
  midiLooper.turnThruOff();
  
  // Sets event handlers
  midiLooper.setHandleControlChange(handleControlChange);
  midiLooper.setHandleProgramChange(handleProgramChange);

  // Prepares MIDI interface for listening
  midiLooper.begin(MIDI_CHANNEL_OMNI);
  
}

void loop() {
  // Keep reading (polling incoming MIDI messages)
  // Return value isn't checked because everything happens in the handlers
  midiLooper.read();
}

