#include "bluetooth_func.h"

#define NUM_DEVICES 8
#define NUM_SENSORS 3

extern uint8_t devicesState[NUM_DEVICES];
extern uint16_t sensorsValue[NUM_SENSORS];

extern const char *prefixes;

BluetoothSerial SerialBT;

void setUpBluetooth() {
    SerialBT.begin("ESP32_Controller");
    Serial.println("Bluetooth is ready, device name: ESP32_Controller.");
}

bool readBluetooth() {
    if (SerialBT.available()) {
        Serial.printf("Free heap before reading input: %d\n", ESP.getFreeHeap());
        String input = SerialBT.readString();
        Serial.printf("Bluetooth received: %s\n", input.c_str());
        stringToStates(input.c_str());
        return true;
    }
    return false;
}

void stringToStates(const char *input) {
        while (*input) {
        const char *prefixPosition = strchr(prefixes, *input);
        if (prefixPosition) {
            int deviceIndex = prefixPosition - prefixes;
            input++;
            devicesState[deviceIndex] = *input - '0';
        }
        input++;
    }
}

void updateBluetooth() {
    if(SerialBT.hasClient()) {
        Serial.printf("Free heap before updating bluetooth: %d\n", ESP.getFreeHeap());
        //SerialBT.println()
        Serial.printf("j%dk%dl%d\n", sensorsValue[0], sensorsValue[1], sensorsValue[2]);
        SerialBT.printf("j%dk%dl%d", sensorsValue[0], sensorsValue[1], sensorsValue[2]);
    }
}
