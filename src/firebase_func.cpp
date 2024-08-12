// firebase_func.cpp
#include "firebase_func.h"

#define NUM_DEVICES 8
#define NUM_SENSORS 3

extern const char *devicesName[];
extern const char *sensorName[];

extern const char *firebasePath;
extern const char *devicesPath;
extern const char *sensorsPath;

extern const char *firebaseHost;
extern const char *firebaseAuth;

extern uint8_t devicesState[NUM_DEVICES];
extern uint16_t sensorsValue[NUM_SENSORS];

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;
FirebaseData firebaseSetData;

void setUpFirebase() {
    config.database_url = firebaseHost;
    config.signer.tokens.legacy_token = firebaseAuth;
    Firebase.reconnectNetwork(true);
    Firebase.reconnectWiFi(true);
    firebaseData.setBSSLBufferSize(1024, 1024);
    Firebase.begin(&config, &auth);
    firebaseData.keepAlive(5, 5, 1);

    char path1[20];
    snprintf(path1, sizeof(path1), "%s%s", firebasePath, devicesPath);
    if (!Firebase.beginStream(firebaseData, path1)) {
        Serial.printf("Failed to begin stream.\nReason: %s\n", firebaseData.errorReason().c_str());
    } else {
        Serial.printf("Successfully began stream with path: %s\n", path1);
    }
}

void readChange() {
    if (Firebase.ready()) {
        if (!Firebase.readStream(firebaseData)) {
            Serial.printf("Failed to read stream.\nReason: %s\n", firebaseData.errorReason().c_str());
        }

        if (firebaseData.streamTimeout()) {
            if (!firebaseData.httpConnected()) {
                Serial.printf("error code: %d, reason: %s\n\n", firebaseData.httpCode(), firebaseData.errorReason().c_str());
            }
            Serial.print("Stream timeout, resuming ...\n");
            Firebase.beginStream(firebaseData, firebasePath);
        }

        if (firebaseData.streamAvailable()) {
            Serial.printf("Data path: %s\n", firebaseData.dataPath().c_str());
            Serial.printf("Data type: %s\n", firebaseData.dataType().c_str());

            if (firebaseData.dataType() == "json") {
                FirebaseJson json = firebaseData.to<FirebaseJson>();
                FirebaseJsonData jsonData;
                for (int i = 0; i < NUM_DEVICES; ++i) {
                    if (json.get(jsonData, devicesName[i])) {
                        devicesState[i] = jsonData.intValue;
                        Serial.printf("%s: %d\n", devicesName[i], devicesState[i]);
                    }
                }
                Serial.println("Received from database");
            }
            if (strcmp(firebaseData.dataType().c_str(), "int") == 0) {
                for (int i = 0; i < NUM_DEVICES; i++) {
                    if (firebaseData.dataPath().endsWith(devicesName[i])) {
                        devicesState[i] = firebaseData.intData();
                        Serial.printf("Updated device %s to %d\n", devicesName[i], devicesState[i]);
                        break;
                    }
                }
            }
        }
    }
}

void updateDatabase() {
    if (Firebase.ready()) {
        for (int i = 0; i < NUM_SENSORS; i++) {
            char path[25] = "\0";
            snprintf(path, sizeof(path), "%s%s/%s", firebasePath, sensorsPath, sensorName[i]);
            Firebase.setInt(firebaseSetData, path, sensorsValue[i]);
        }
    }
}
