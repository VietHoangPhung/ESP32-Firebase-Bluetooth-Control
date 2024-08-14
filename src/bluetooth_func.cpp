#include "bluetooth_func.h"

BluetoothSerial SerialBT;

const char *prefixes = "abcdefgh";
const char *nn = "xyz";

void setUpBluetooth() {
    SerialBT.begin("ESP32_Controller");
    Serial.println("Bluetooth is ready, device name: ESP32_Controller.");
}


bool readBluetooth() {
    if (SerialBT.available()) {
        Serial.printf("Free heap before reading input: %d\n", ESP.getFreeHeap());
        char inputBuffer[50];
        int index = 0;

        while (SerialBT.available() && index < sizeof(inputBuffer) - 1) {
            inputBuffer[index++] = SerialBT.read();
        }
        inputBuffer[index] = '\0'; // Null-terminate the string

        Serial.printf("Bluetooth received: %s\n", inputBuffer);
        stringToStates(inputBuffer, devicesState);
        return true;
    }
    return false;
}

void stringToStates(const char *input, uint8_t *pDevicesState) {
    while (*input) {
        const char *prefixPosition = strchr(prefixes, *input);
        if (prefixPosition) {
            int deviceIndex = prefixPosition - prefixes;
            input++;
            pDevicesState[deviceIndex] = *input - '0';
        }
        input++;
    }
}

void updateBluetooth() {
    char outputBuffer[50] = {0};
    uint16_t *pSensorsValue = sensorsValue;
    int index = 0;                                                                                                                                                                                                                              

    for (int i = 0; i < NUM_SENSORS; i++) {
        index += snprintf(outputBuffer + index, sizeof(outputBuffer) - index, "%c%d ", nn[i], pSensorsValue[i]);
    }

    SerialBT.println(outputBuffer);
}
