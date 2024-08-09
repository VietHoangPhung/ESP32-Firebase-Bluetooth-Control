#include <Arduino.h>
#include "FirebaseESP32.h"
#include <BluetoothSerial.h>
#include <WiFi.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  Serial.begin(112500);

  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.printf("Result: %d\n", result);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}