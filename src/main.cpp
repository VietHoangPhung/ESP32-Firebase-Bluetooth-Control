/*/home     path        prefix        name
  -> /devices
            /door       a             rolling door
            /gate       b             gate door
            /light1     c             street light
            /light2     d             stair light
            /light3     e             bedroom light   
            /light4     f             kitchen light
            /pump       g             water pump
            /rack       h             drying rack
  -> /sensors   
            /gas        no            gas sensor
            /hum        no            humidity sensor
            /temp       no            temperature sensor
            
*/
#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include "firebase_func.h"

#define LED 2
#define BTN 4
#define NUM_DEVICES 8
#define NUM_SENSORS 3
#define NUM_WARNINGS 2

//const char* firebasePath = "/home";

const char *ssid = "git";
const char *password = "commit";
const char *devicesName[] = {"door", "gate", "light1", "light2", "light3", "light4", "pump", "rack"};
const char *sensorName[] = {"gas", "hum", "temp"};
const char *prefixes = "abcdefghjkl";

const char *firebasePath = "/home";
const char *devicesPath = "/devices";
const char *sensorsPath = "/sensors";

const char *firebaseHost = "git-commit-default-rtdb.firebaseio.com";
const char *firebaseAuth = "d2AnlPYBY9lWl0sN57CITh8EmpHxi479n0DR5hEH";

uint8_t devicesState[NUM_DEVICES];
uint16_t sensorsValue[NUM_SENSORS];

unsigned long lastSet, lastGet;

// FirebaseConfig config;
// FirebaseAuth auth;
// FirebaseData firebaseData;

void handleFirebase(void *parameter);
void handleBluetooth(void *parameter);
void connectWifi();
void updateDevices();

extern void setUpFirebase();
extern void readChange();
extern void updateDatabase();


void setup() {
  Serial.begin(112500);
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);

  xTaskCreatePinnedToCore(
    handleBluetooth,
    "BluetoothTask",
    10000,
    NULL,
    1,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    handleFirebase,
    "FirebaseTask",
    10000,
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
  Serial.println("Connecting to Wi-Fi ...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    if ((unsigned long)(millis() - startTime) > 10000) {
      Serial.println("Please check your Wi-Fi. If it takes too long, try rebooting ...\nRestarting connection.\n");
      startTime = millis();
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }
  Serial.printf("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void handleFirebase(void *parameter) {
  connectWifi();
  setUpFirebase();
  while (true) {
    readChange();

    if ((unsigned long)(millis() - lastSet) > 5000) {
      sensorsValue[0] = random(0, 100);
      Serial.printf("Start: %ld, sending %d\n", millis(), sensorsValue[0]);
      updateDatabase();
      Serial.printf("Finish: %ld, sent %d\n", millis(), sensorsValue[0]);
      lastSet = millis();
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void handleBluetooth(void *parameter) {
  while (true) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
