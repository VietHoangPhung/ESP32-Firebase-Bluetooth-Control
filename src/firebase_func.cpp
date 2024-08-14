// firebase_func.cpp

/**/
#include "firebase_func.h"

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;
FirebaseData firebaseSetData;

const char *devicesName[] = {"door", "gate", "light1", "light2", "light3", "light4", "pump", "rack"};
const char *sensorsName[] = {"gas", "hum", "temp"};

const char *firebaseHost = "git-commit-default-rtdb.firebaseio.com";
const char *firebaseAuth = "gitcommit";

const char *devicesPath = "/devices";
const char *sensorsPath = "/sensors";

void setUpFirebase() {
    config.database_url = firebaseHost;
    config.signer.tokens.legacy_token = firebaseAuth;
    Firebase.reconnectNetwork(true);
    Firebase.reconnectWiFi(true);
    firebaseData.setBSSLBufferSize(4096, 512);
    firebaseSetData.setBSSLBufferSize(4096, 512);
    Firebase.begin(&config, &auth);
    //firebaseData.keepAlive(8, 8, 1);

    if (!Firebase.beginStream(firebaseData, devicesPath)) {
        //Serial.printf("Failed to begin stream.\nReason: %s\n", firebaseData.errorReason().c_str());
    } else {
        //Serial.printf("Successfully began stream with path: %s\n", devicesPath);
    }
}

bool readChange() {
    if (Firebase.ready()) {
        if (!Firebase.readStream(firebaseData)) {
            //Serial.printf("Failed to read stream.\nReason: %s\n", firebaseData.errorReason().c_str());
        }

        if (firebaseData.streamTimeout()) {
            if (!firebaseData.httpConnected()) {
                //Serial.printf("error code: %d, reason: %s\n\n", firebaseData.httpCode(), firebaseData.errorReason().c_str());
            }
            //Serial.print("Stream timeout, resuming ...\n");
            //Firebase.beginStream(firebaseData, devicesPath);
        }

        if (firebaseData.streamAvailable()) {
            //Serial.printf("Data path: %s\n", firebaseData.dataPath().c_str());
            //Serial.printf("Data type: %s\n", firebaseData.dataType().c_str());

            // if (firebaseData.dataType() == "json") {
            //     FirebaseJson json = firebaseData.to<FirebaseJson>();
            //     FirebaseJsonData jsonData;
            //     //uint8_t *pDevicesState;
            //     for (int i = 0; i < NUM_DEVICES; ++i) {
            //         if (json.get(jsonData, devicesName[i])) {
            //             devicesState[i] = jsonData.intValue;
            //             Serial.printf("%s: %d\n", devicesName[i], devicesState[i]);
            //         }
            //     }
            //     return true;
            // } else 
            if (firebaseData.dataType() == "int") {
                for (int i = 0; i < NUM_DEVICES; i++) {
                    if (firebaseData.dataPath().endsWith(devicesName[i])) {
                        devicesState[i] = firebaseData.intData();
                        //Serial.printf("Updated device %s to %d\n", devicesName[i], devicesState[i]);
                    }
                }
                return true;
            }
        }
    }
    return false;
}

void updateDatabase() {
    if (Firebase.ready()) {
        FirebaseJson json;
        
        // Populate the JSON object with sensor values
        for (int i = 0; i < NUM_SENSORS; i++) {
            json.set(sensorsName[i], sensorsValue[i]);
        }

        // Set the entire JSON object in one operation
        Serial.printf("Started updating: %d. Time: %ld\n", sensorsValue[0], millis());
        if (Firebase.set(firebaseSetData, sensorsPath, json)) {
            Serial.println("Sensors data updated successfully.");
        } else {
            Serial.printf("Failed to update sensors data. Reason: %s\n", firebaseSetData.errorReason().c_str());
        }
        Serial.printf("Finished updating: %d. Time: %ld\n", sensorsValue[0], millis());
    }
}
