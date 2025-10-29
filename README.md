# ESP32_AP_Station_WiFiProvisioning
ESP32 project demonstrating simultaneous Access Point (AP) and Station (STA) mode with Wi-Fi provisioning, allowing dynamic network configuration for IoT applications.
1. Project Overview

This project shows how an ESP32 can operate in dual Wi-Fi mode (AP + STA) while providing a Wi-Fi provisioning interface:

Station Mode: Connects to an existing Wi-Fi network.

Access Point Mode: Hosts a network for provisioning/configuration.

Users can set or update Wi-Fi credentials through the AP without hardcoding them.
This is useful for IoT devices where network details may change or need easy configuration in the field.

2. Features

Dual-mode Wi-Fi (AP + STA)

Dynamic Wi-Fi provisioning via captive portal or configuration page

Automatic reconnection if Wi-Fi credentials change

Supports asynchronous connection handling and data buffering

3. Hardware Requirements

ESP32 development board

USB cable for programming

Optional: Sensors or peripherals for testing IoT functionality

4. How to Use

Unzip the provided code folder.

Open the Arduino IDE (or PlatformIO) and load the main .ino file.

Upload to ESP32.

Connect to the ESP32 AP to configure Wi-Fi credentials.

Observe the device connecting to your Wi-Fi network automatically while still hosting its AP.
