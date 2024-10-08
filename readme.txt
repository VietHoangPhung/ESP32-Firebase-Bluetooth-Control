ESP32-Firebase-Bluetooth-Control

Using PlatformIO

This project is a part of (update later).

Overview
This project uses an ESP32 to control devices, demonstrated with the built-in LED on pin 2.

Features
- Firebase Real-Time Database:

+ Listens for changes in the database and updates the local device state accordingly.
+ Continuously uploads sensor values to the database every 5 seconds.
- Bluetooth:

+ Listens for incoming Bluetooth data, decodes it, and updates device states based on the input.
+ Continuously sends sensor values via Bluetooth every 5 seconds if a client is connected.
- EEPROM:

+ Writes the device states to EEPROM when changes occur.
+ Reads the device states from EEPROM on boot and updates the local device states.