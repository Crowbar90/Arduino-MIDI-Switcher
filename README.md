# Arduino-MIDI-Switcher
MIDI Switcher based on Arduino Uno R3 board.

This project contains code for a device that listens for incoming MIDI messages and outputs a byte through a shift register. The shift register outputs can be used to drive various pieces of equipment (for instance, driving suitable relay coils allows to create a looper or drive simple switches via MIDI).

This implementation uses a 74HC595 shift register in order to have a separate output for each bit. This way "mixed" devices can be implemented (like a device with 4 loops for effect pedals and 4 switches).

The device listens to Program Change messages (which load a previously stored output sequence) and specific Control Change numbers (specified in Switcher/Switcher.ino), which change a single bit of the output sequence. Another Control Change message can be sent to the device to store the current output on a specific Program Change.

The device currently listens on all MIDI channels. It's possible to make it listen only on a specific channel in the code, and in the future selection of the channel will be implemented on a dip switch (this way it will be possible to chain several devices using the same software, with no need to change the channel manually).
