#include "hc05.h"
#include <Arduino.h>

HardwareSerial BTSerial(0);

//const char *prefixes = "abcdefgh";
//const char *nn = "xyz";

void setUpHc05() {
  // Start serial communication with the HC-05 module
    BTSerial.begin(9600, SERIAL_8N1, 3, 1); // 9600 is the default baud rate for HC-05
  // RX pin is GPIO 3, TX pin is GPIO 1

  Serial.println("ESP32 HC-05 Bluetooth Communication");
}


bool readHc05() {
    if (BTSerial.available()) {
        //Serial.printf("Free heap before reading input: %d\n", ESP.getFreeHeap());
        char inputBuffer[50];
        int index = 0;

        while (BTSerial.available() && index < sizeof(inputBuffer) - 1) {
            inputBuffer[index++] = BTSerial.read();
        }
        inputBuffer[index] = '\0';
        //unsigned int heap = ESP.getFreeHeap();
        BTSerial.printf("Echo: %s, freeH heap: %d\n", inputBuffer, ESP.getFreeHeap());
        stringToStatesHc05(inputBuffer, devicesState);
        return true;
    }
    return false;
}

// assign devices' state based on input string
void stringToStatesHc05(const char *input, uint8_t *pDevicesState) {
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


// update sensors' value to the app via bluetooth
void updateHc05() {
    char outputBuffer[50] = {0};
    uint16_t *pSensorsValue = sensorsValue;
    int index = 0;                                                                                                                                                                                                                              

    for (int i = 0; i < NUM_SENSORS; i++) {
        index += snprintf(outputBuffer + index, sizeof(outputBuffer) - index, "%c%d ", nn[i], pSensorsValue[i]);
    }

    BTSerial.println(outputBuffer);
}
