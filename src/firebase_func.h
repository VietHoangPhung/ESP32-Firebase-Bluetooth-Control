// firebase_func.h
#ifndef FIREBASE_FUNC_H
#define FIREBASE_FUNC_H

#include <FirebaseESP32.h>

extern const char *devicesName[];
extern const char *sensorName[];
extern const char *firebasePath;

extern const char *firebaseHost;
extern const char *firebaseAuth;

extern uint8_t devicesState[];
extern uint16_t sensorsValue[];
extern FirebaseConfig config;
extern FirebaseAuth auth;
extern FirebaseData firebaseData;

// Declare the functions
void setUpFirebase();
void readChange();
void updateDatabase();

#endif // FIREBASE_FUNC_H