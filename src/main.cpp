/*          path        prefix        name
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
#include "bluetooth_func.h"

#define LED 2
#define BTN 4
#define NUM_DEVICES 8
#define NUM_SENSORS 3
#define NUM_WARNINGS 2

//const char* firebasePath = "/home";

const char *ssid = "Ximiao";
const char *password = "12345679";
const char *devicesName[] = {"door", "gate", "light1", "light2", "light3", "light4", "pump", "rack"};
const char *sensorsName[] = {"gas", "hum", "temp"};
const char *prefixes = "abcdefgh";

//const char *firebasePath = "/home";
const char *devicesPath = "/devices";
const char *sensorsPath = "/sensors";

const char *firebaseHost = "git-commit-default-rtdb.firebaseio.com";
const char *firebaseAuth = "gitcommit";

uint8_t devicesState[NUM_DEVICES];
uint16_t sensorsValue[NUM_SENSORS];

unsigned long lastSet, lastGet;
//BluetoothSerial SerialBT;

// FirebaseConfig config;
// FirebaseAuth auth;
// FirebaseData firebaseData;

void handleFirebase(void *parameter);
void handleBluetooth(void *parameter);
void connectWifi();
void updateDevices();

extern void setUpFirebase();
extern bool readChange();
extern void updateDatabase();

extern void setUpBluetooth();
extern bool readBluetooth();
extern void stringToStates(const char*);


void setup() {
  Serial.begin(112500);
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);

  xTaskCreatePinnedToCore(
    handleBluetooth,
    "BluetoothTask",
    4096,
    NULL,
    1,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    handleFirebase,
    "FirebaseTask",
    10240,
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
  Serial.printf("Free heap before WiFi and Firebase initialization: %d\n", ESP.getFreeHeap());
  connectWifi();
  setUpFirebase();
  while (true) {
    if(readChange()) {
      Serial.printf("Free heap after reading change: %d\n", ESP.getFreeHeap());
      updateDevices();
    }
    if (WiFi.status() == WL_CONNECTED && (unsigned long)(millis() - lastSet) > 5000) {
      Serial.printf("Free heap before updating database: %d\n", ESP.getFreeHeap());
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
  Serial.printf("Free heap before Bluetooth initialization: %d\n", ESP.getFreeHeap());

  if (ESP.getFreeHeap() > 50000) { // Ensure sufficient memory is available
    setUpBluetooth();
  } else {
    Serial.println("Not enough memory to initialize Bluetooth.\n");
  }


  while (true) {
    if(readBluetooth()) {
      Serial.printf("Free heap before updating devices: %d\n", ESP.getFreeHeap());
      updateDevices();
    } 
    if ((unsigned long)(millis() - lastGet) > 2000) {
      updateBluetooth();
      lastGet = millis();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void updateDevices() {
  digitalWrite(LED, devicesState[2]);
  for (int i = 0; i < NUM_DEVICES; i++) {
    Serial.printf("%d ", devicesState[i]);
  }
  Serial.println();
}
