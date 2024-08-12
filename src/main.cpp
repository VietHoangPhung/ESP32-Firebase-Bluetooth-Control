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

const char *ssid = "Ximiao";
const char *password = "12345679";
const char *devicesName[] = {"door", "gate", "light1", "light2", "light3", "light4", "pump", "rack"};
const char *sensorName[] = {"gas", "hum", "temp"};
const char *prefixes = "abcdefghjkl";

//const char *firebasePath = "/home";
const char *devicesPath = "/devices";
const char *sensorsPath = "/sensors";

const char *firebaseHost = "controller-bcf36-default-rtdb.firebaseio.com";
const char *firebaseAuth = "d2AnlPYBY9lWl0sN57CITh8EmpHxi479n0DR5hEH";

uint8_t devicesState[NUM_DEVICES];
uint16_t sensorsValue[NUM_SENSORS];

unsigned long lastSet, lastGet;
BluetoothSerial SerialBT;

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


void setup() {
  Serial.begin(112500);
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);

  xTaskCreatePinnedToCore(
    handleBluetooth,
    "BluetoothTask",
    2048,
    NULL,
    1,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    handleFirebase,
    "FirebaseTask",
    8192,
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
    if(readChange()) {
      updateDevices();
    }

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
  Serial.printf("Free heap before Bluetooth initialization: %d\n", ESP.getFreeHeap());

  //if (ESP.getFreeHeap() > 50000) { // Ensure sufficient memory is available
  SerialBT.begin("ESP32_BT");  // Bluetooth device name
  Serial.println("Bluetooth device name: ESP32_BT\n");
  // } else {
  //   Serial.println("Not enough memory to initialize Bluetooth.\n");
  // }

  while (true) {
    if (SerialBT.available()) {
      String input = SerialBT.readString();
      Serial.printf("Received message: %s\n", input);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void updateDevices() {
  digitalWrite(LED, devicesState[2]);
  for (int i = 0; i < NUM_DEVICES; i++) {
    Serial.printf("%d, ", devicesState[i]);
  }
  Serial.println();
}
