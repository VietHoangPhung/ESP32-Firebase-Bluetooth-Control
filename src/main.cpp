/*
    database path       bluetooth prefix        name                     state/values
                                                                    on/open  off/close  auto
/devices
    /door               a                       rolling door        1        0
    /gate               b                       gate door rfid      1        0
    /light1             c                       street light        1        0
    /light2             d                       stair light         1        0
    /light3             e                       bedroom light       1        0
    /light4             f                       kitchen light       1        0
    /pump               g                       water pump          1        0
    /rack               h                       drying rack         1        0          2
/sensors
    /gas                x
    /hum                y
    /temp               x

*/

#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include "firebase_func.h"
//#include "bluetooth_func.h"
#include "hc05.h"


#define LED 2
#define NUM_DEVICES 8
#define NUM_SENSORS 3
#define NUM_WARNINGS 2

const char *ssid = "Ximiao";
const char *password = "12345679";

uint8_t devicesState[NUM_DEVICES];
uint16_t sensorsValue[NUM_SENSORS];

unsigned long preDbUpdate, preBtUpdate;

void handleFirebase(void *parameter);
void handleBluetooth(void *parameter);
//void handleUart(void *parameter);
void connectWifi();
void updateDevices();
void storeStates();

extern void setUpFirebase();
extern bool readChange();
extern void updateDatabase();

extern void setUpHc05();
extern bool readHc05();
extern void stringToStatesHc05(const char*, uint8_t*);

void setup() {
    //Serial.begin(112500);
    pinMode(LED, OUTPUT);
    EEPROM.begin(NUM_DEVICES);
    for (int i = 0; i < NUM_DEVICES; i++) {
        devicesState[i] = EEPROM.read(i);
        //Serial.printf("%d ", devicesState[i]);
    }
    //Serial.println();
    updateDevices();

    xTaskCreatePinnedToCore(
        handleBluetooth,
        "BluetoothTask",
        3072,
        NULL,
        1,
        NULL,
        0
    );

    // xTaskCreatePinnedToCore(
    //     handleUart,
    //     "UartTask",
    //     1024,
    //     NULL,
    //     1,
    //     NULL,
    //     0
    // );

    xTaskCreatePinnedToCore(
        handleFirebase,
        "FirebaseTask",
        6144,
        NULL,
        1,
        NULL,
        1
    );
}

void loop() {
}

void connectWifi() {
    unsigned long startTime = millis();
    //Serial.println("Connecting to Wi-Fi ...");
    WiFi.begin(ssid, password);

    // restart connecttin process if it takes too long 
    while (WiFi.status() != WL_CONNECTED) {
        if ((unsigned long)(millis() - startTime) > 10000) {
            //Serial.println("Please check your Wi-Fi. If it takes too long, try rebooting ...\nRestarting connection.\n");
            startTime = millis();
            WiFi.disconnect();
            WiFi.begin(ssid, password);
        }
    }
    //Serial.printf("Connected with IP: ");
    //Serial.println(WiFi.localIP());
}

void handleFirebase(void *parameter) {
    //Serial.printf("Free heap before WiFi and Firebase initialization: %d\n", ESP.getFreeHeap());
    connectWifi();
    setUpFirebase();
    while (true) {
        if (readChange()) {
            //Serial.printf("Free heap after reading change: %d\n", ESP.getFreeHeap());
            storeStates();
            updateDevices();
        }
        if (WiFi.status() == WL_CONNECTED && (unsigned long)(millis() - preDbUpdate) > 5000) {
            //Serial.printf("Free heap before updating database: %d\n", ESP.getFreeHeap());
            //Serial.printf("Start: %ld, sending %d\n", millis(), sensorsValue[0]);
            updateDatabase();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            //Serial.printf("Finish: %ld, sent %d\n", millis(), sensorsValue[0]);
            preDbUpdate = millis();
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void handleBluetooth(void *parameter) {
    //Serial.printf("Free heap before Bluetooth initialization: %d\n", ESP.getFreeHeap());

    // if (ESP.getFreeHeap() > 50000) { // Ensure sufficient memory is available
        setUpHc05();
    // } else {
    //     //Serial.println("Not enough memory to initialize Bluetooth.\n");
    // }
    Serial.printf("Free heap after Bluetooth initialization: %d\n", ESP.getFreeHeap());
    while (true) {
        if (readHc05()) {
            //Serial.printf("Free heap before updating devices: %d\n", ESP.getFreeHeap());
            storeStates();
            updateDevices();
        }
        if ((unsigned long)(millis() - preBtUpdate) > 2000) {
            sensorsValue[0] = random(0, 100);
            sensorsValue[1] = random(0, 100);
            sensorsValue[2] = random(0, 100);
    
            updateHc05();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            preBtUpdate = millis();
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

// void handleUart(void *parameter) {
//     while (true) {
//         vTaskDelay(200 / portTICK_PERIOD_MS);
//     }
// }

// Update devices based on devices' state
void updateDevices() {
    digitalWrite(LED, devicesState[2]);
    for (int i = 0; i < NUM_DEVICES; i++) {
        //Serial.printf("%d ", devicesState[i]);
    }
    //Serial.println();
}


// Store devices' state in eeprom
void storeStates() {
    EEPROM.begin(NUM_DEVICES);
    for (int i = 0; i < NUM_DEVICES; i++) {
        EEPROM.write(i, devicesState[i]);
    }
    EEPROM.commit();
}
