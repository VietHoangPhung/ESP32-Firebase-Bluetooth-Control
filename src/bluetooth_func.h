// bluetooth_func.h
#ifndef BLUETOOTH_FUNC_H
#define BLUETOOTH_FUNC_H

#include <BluetoothSerial.h>

extern uint8_t devicesState[];
extern uint16_t sensorsValue[];
extern const char *prefixes;

// Declare the functions
void setUpBluetooth();
bool readBluetooth();
void stringToStates(const char*);
void updateBluetooth();

#endif // BLUETOOTH_FUNC_H