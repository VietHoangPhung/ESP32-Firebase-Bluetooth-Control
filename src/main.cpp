#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include "firebase_func.h"
#include "bluetooth_func.h"


#define LED 2
#define BTN 4
#define NUM_DEVICES 8
#define NUM_SENSORS 3
#define NUM_WARNINGS 2

const char *ssid = "Ximiao";
const char *password = "12345679";

uint8_t devicesState[NUM_DEVICES];
uint16_t sensorsValue[NUM_SENSORS];

unsigned long lastSet, lastGet;

void handleFirebase(void *parameter);
void handleBluetooth(void *parameter);
//void handleUart(void *parameter);
void connectWifi();
void updateDevices();
void storeStates();

extern void setUpFirebase();
extern bool readChange();
extern void updateDatabase();

extern void setUpBluetooth();
extern bool readBluetooth();
extern void stringToStates(const char*, uint8_t*);

void setup() {
    Serial.begin(112500);
    pinMode(LED, OUTPUT);
    pinMode(BTN, INPUT_PULLUP);
    EEPROM.begin(NUM_DEVICES); // Initialize EEPROM with the size of the array
    for (int i = 0; i < NUM_DEVICES; i++) {
        devicesState[i] = EEPROM.read(i);
        Serial.printf("%d ", devicesState[i]);
    }
    Serial.println();
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
    // No code in loop to prioritize task management
}

void connectWifi() {
    unsigned long startTime = millis();
    Serial.println("Connecting to Wi-Fi ...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        if ((unsigned long)(millis() - startTime) > 10000) {
            //Serial.println("Please check your Wi-Fi. If it takes too long, try rebooting ...\nRestarting connection.\n");
            startTime = millis();
            WiFi.disconnect();
            WiFi.begin(ssid, password);
        }
    }
    //Serial.printf("Connected with IP: ");
    Serial.println(WiFi.localIP());
}

void handleFirebase(void *parameter) {
    Serial.printf("Free heap before WiFi and Firebase initialization: %d\n", ESP.getFreeHeap());
    connectWifi();
    setUpFirebase();
    while (true) {
        if (readChange()) {
            Serial.printf("Free heap after reading change: %d\n", ESP.getFreeHeap());
            storeStates();
            updateDevices();
        }
        if (WiFi.status() == WL_CONNECTED && (unsigned long)(millis() - lastSet) > 5000) {
            Serial.printf("Free heap before updating database: %d\n", ESP.getFreeHeap());
            sensorsValue[0] = random(0, 100);
            sensorsValue[1] = random(0, 100);
            sensorsValue[2] = random(0, 100);
            //Serial.printf("Start: %ld, sending %d\n", millis(), sensorsValue[0]);
            updateDatabase();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            //Serial.printf("Finish: %ld, sent %d\n", millis(), sensorsValue[0]);
            lastSet = millis();
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void handleBluetooth(void *parameter) {
    Serial.printf("Free heap before Bluetooth initialization: %d\n", ESP.getFreeHeap());

    if (ESP.getFreeHeap() > 50000) { // Ensure sufficient memory is available
        setUpBluetooth();
    } else {
        //Serial.println("Not enough memory to initialize Bluetooth.\n");
    }
    Serial.printf("Free heap after Bluetooth initialization: %d\n", ESP.getFreeHeap());
    while (true) {
        if (readBluetooth()) {
            Serial.printf("Free heap before updating devices: %d\n", ESP.getFreeHeap());
            storeStates();
            updateDevices();
        }
        if ((unsigned long)(millis() - lastGet) > 2000) {
            updateBluetooth();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            lastGet = millis();
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

// void handleUart(void *parameter) {
//     while (true) {
//         vTaskDelay(200 / portTICK_PERIOD_MS);
//     }
// }

void updateDevices() {
    digitalWrite(LED, devicesState[2]);
    for (int i = 0; i < NUM_DEVICES; i++) {
        Serial.printf("%d ", devicesState[i]);
    }
    Serial.println();
}

void storeStates() {
    EEPROM.begin(NUM_DEVICES);
    for (int i = 0; i < NUM_DEVICES; i++) {
        EEPROM.write(i, devicesState[i]);
    }
    EEPROM.commit();
}
