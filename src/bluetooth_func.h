// bluetooth_func.h
#ifndef BLUETOOTH_FUNC_H
#define BLUETOOTH_FUNC_H

#include <BluetoothSerial.h>

#define NUM_DEVICES 8
#define NUM_SENSORS 3

extern BluetoothSerial SerialBT;

extern uint8_t devicesState[NUM_DEVICES];
extern uint16_t sensorsValue[NUM_SENSORS];
extern const char *prefixes;
extern const char *nn;

// Declare the functions
void setUpBluetooth();
bool readBluetooth();
void stringToStates(const char*, uint8_t*);
void updateBluetooth();

#endif // BLUETOOTH_FUNC_H