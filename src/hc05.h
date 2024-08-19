// hc05.h

#ifndef HC05_H
#define HC05_H
#include <HardwareSerial.h>

#define NUM_DEVICES 8
#define NUM_SENSORS 3

extern uint8_t devicesState[NUM_DEVICES];
extern uint16_t sensorsValue[NUM_SENSORS];
extern const char *prefixes;
extern const char *nn;

// Declare the functions
void setUpHc05();
bool readHc05();
void stringToStatesHc05(const char*, uint8_t*);
void updateHc05();

#endif // HC05_H