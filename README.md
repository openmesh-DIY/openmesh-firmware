# openmesh-firmware

OpenMesh is a cheap, Arduino-based LoRa mesh firmware for ESP32 + SX127x radios.

It is designed for:
- DIY hardware
- Low-cost nodes
- Full user control
- Bluetooth Classic configuration
- OLED-based standalone messaging

## Goals
- No vendor lock-in  
- No expensive boards  
- Arduino-first  
- Works without a phone  
- Phone optional via Bluetooth Classic  

## Features
- ESP32 (WROOM32 recommended)
- SX1276 / SX1278 support
- LoRa CSS modulation
- Mesh routing with TTL
- Anti-loop protection
- AES-256 encryption
- Bluetooth Classic (SPP) control
- OLED UI for quick messages
- Broadcast and directed messaging

## Status
⚠️ **Experimental** — under active development.  
Expect bugs, breaking changes, and rapid iteration.

## Hardware
- ESP32 (WROOM32 recommended)
- SX1276 / SX1278 LoRa module
- 128x64 OLED (SSD1306)
- 433 based

## Contributing
Contributions are welcome.

By contributing to this project, you agree that your contributions may be
used, modified, and **relicensed** under future versions of this project’s
license by the OpenMesh maintainers.

## License
Licensed under the **Apache License, Version 2.0**.
See the `LICENSE` file for details.
