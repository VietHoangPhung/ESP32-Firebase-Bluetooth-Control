#include <Arduino.h>
#include <WiFi.h>
#include "FirebaseESP32.h"
#include <BluetoothSerial.h>

#define LED_PIN 2
#define BTN_PIN 4
#define NUM_DEVICES 7

BluetoothSerial SerialBT;

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

uint8_t states[NUM_DEVICES] = {0};
uint8_t prevStates[NUM_DEVICES] = {0};

const char* devicesName[] = {"door", "gas", "hum", "light1", "pump", "rack", "temp"};
const char* prefixes = "dghlprt";

uint8_t ledState = 0;
bool bluetoothNeedUpdate = false;
bool firebaseNeedUpdate = false;

bool bluetoothJustUpdated = false;
bool firebaseJustUpdated = false;

const char* firebasePath = "/home";
const char* testPath = "/home/light1";
const char* ssid = "viethoang-2.4Ghz";// change this to your wifi ssid
const char* password = "12345679"; // and this to your password
const char* firebaseHost = "controller-bcf36-default-rtdb.firebaseio.com";
const char* firebaseAuth = "d2AnlPYBY9lWl0sN57CITh8EmpHxi479n0DR5hEH";

void handleFirebase(void *parameter);
void handleBluetooth(void *parameter);

void connectWiFi();
void setUpFirebase();
void updateDevices();
void updateFirebase();
//void updateBluetooth();

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.begin(115200);

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
  // put your main code here, to run repeatedly:

}

void connectWiFi() {
  unsigned long startAttemptTime = millis();
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    // Restart connection if it takes too long (6 seconds)
    if ((unsigned long)(millis() - startAttemptTime) >= 6000) {
      Serial.println("\nConnection taking too long, restarting...");
      startAttemptTime = millis();
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }
}

void setUpFirebase() {
  config.database_url = firebaseHost;
  config.signer.tokens.legacy_token = firebaseAuth;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(firebaseData, firebasePath)) {
    Serial.printf("Could not begin stream for %s\n",  firebasePath);
    Serial.printf("REASON: %s\n", firebaseData.errorReason());
  } else {
    Serial.printf("Successfully began stream for %s\n", firebasePath);
  }
}

void handleFirebase(void *parameter) {
  connectWiFi();
  setUpFirebase();
  //String path = String(firebasePath) + "/" + devicesName[3];
  unsigned long lastUpdate = millis();

  while (true) {
    if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
      if (!Firebase.readStream(firebaseData)) {
        Serial.println("Stream read failed");
        Serial.printf("REASON: %s\n", firebaseData.errorReason());
      }

      if (firebaseData.streamTimeout()) {
        Serial.println("Stream timeout, resuming...");
        Firebase.beginStream(firebaseData, firebasePath);
      }

      if (firebaseData.streamAvailable() && !firebaseJustUpdated) {
        if (firebaseData.dataType() == "json") {
          FirebaseJson json = firebaseData.to<FirebaseJson>();
          FirebaseJsonData jsonData;
          for (int i = 0; i < NUM_DEVICES; ++i) {
            if (json.get(jsonData, devicesName[i])) {
              states[i] = jsonData.intValue;
              bluetoothNeedUpdate = true;  // Set flag to update Bluetooth
            }
          }
          Serial.printf("Received from database\n");
        } else if (firebaseData.dataType() == "int") {
          String path = firebaseData.dataPath();
          Serial.printf("Received from path: %s\n", path);
          for (int i = 0; i < NUM_DEVICES; ++i) {
            if (path.endsWith(devicesName[i])) {
              states[i] = firebaseData.intData();
              bluetoothNeedUpdate = true;  // Set flag to update Bluetooth
              Serial.printf("Received from database, device: %s, value %d\n", devicesName[i], states[i]);
              break;
            }
          }
        }
        updateDevices();
      } else if (firebaseJustUpdated) {
        // Add a small delay to ensure the update is processed
        vTaskDelay(200 / portTICK_PERIOD_MS);
        firebaseJustUpdated = false;
      }

      //if ((unsigned long)(millis() - lastUpdate) > 5000) {
        if (firebaseNeedUpdate) {
          Serial.printf("Updating %s 1 value: %d\n",testPath, states[3]);
          if (!Firebase.setInt(firebaseData, testPath, ledState)) {
            Serial.printf("Failed to update: %s\n", firebaseData.errorReason());
          } else {
            Serial.println("Successfully updated ");
            firebaseNeedUpdate = false;
            firebaseJustUpdated = true;
          }
        }
      //}

    } else {
      Serial.println("Wi-Fi disconnected. Reconnecting...");
      connectWiFi();
      setUpFirebase();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void handleBluetooth(void *parameter) {
  SerialBT.begin("ESP32_BLT");
  Serial.println("Device name: ESP32_BLT");
  while(true) {
    if(SerialBT.available()) {
      String input = SerialBT.readString();
      Serial.printf("bluetooth received: %s\n", input);
      char prefix = input.charAt(0);
      int value = (input.substring(1)).toInt();
      if (prefix == 'l') {
        states[3] = value;
        firebaseNeedUpdate = true;
      }
      updateDevices();
    }

    if (bluetoothNeedUpdate && SerialBT.hasClient()) {
      String message = "l";
      message.concat(states[3]);
      SerialBT.println(message);
      Serial.printf("sent via bluetooth: %s\n", message);
      bluetoothNeedUpdate = false;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void updateDevices() {
  digitalWrite(LED_PIN, states[3]);
}