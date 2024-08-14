// firebase_func.h
#ifndef FIREBASE_FUNC_H
#define FIREBASE_FUNC_H
#define NUM_DEVICES 8
#define NUM_SENSORS 3

#include <FirebaseESP32.h>

extern FirebaseConfig config;
extern FirebaseAuth auth;
extern FirebaseData firebaseData;
extern FirebaseData firebaseSetData;

extern const char *devicesName[];
extern const char *sensorsName[];

extern const char *firebaseHost;
extern const char *firebaseAuth;

extern const char *devicesPath;
extern const char *sensorsPath;

extern uint8_t devicesState[NUM_DEVICES];
extern uint16_t sensorsValue[NUM_SENSORS];

void setUpFirebase();
bool readChange();
void updateDatabase();

#endif // FIREBASE_FUNC_H