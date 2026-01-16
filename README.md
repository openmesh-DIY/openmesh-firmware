# OpenMesh Firmware

OpenMesh is a **cheap, DIY, Arduino-first LoRa mesh firmware** for ESP32 + SX127x radios.

This project exists so **normal people** can build mesh comms without:
- expensive boards
- vendor lock-in
- forced GPS
- forced cloud
- forced apps

Notes:
> OpenMesh is not perfect. Neither are you. That's fine.

Phone is **optional**.  
Nodes work **standalone** with OLED + buttons.

---

## 🔥 Core Principles

- Cheap hardware
- Arduino IDE friendly
- Full user control
- Bluetooth Classic (SPP) configuration
- Works without internet
- Works without phone
- Open protocol

---

## ✨ Features

- ESP32 (WROOM32 recommended)
- SX1276 / SX1278 LoRa (CSS)
- Broadcast & Directed messages
- Mesh routing with TTL (hop count)
- Anti-loop protection
- Node naming stored in flash
- Bluetooth Classic (SPP) CLI
- OLED UI (SSD1306)
- AES-256 encryption (optional)
- No GPS
- No vendor servers

---

## 🧱 Hardware

Minimum:
- ESP32
- SX1276 / SX1278
- OLED SSD1306 (128x64)

Supported bands:
- 433 MHz
- 868 MHz
- 915 MHz

---

## 📡 How It Works (Simple)

- Every node has:
  - Node ID
  - Human-readable node name
- Messages contain:
  - SRC
  - DEST (or broadcast)
  - TTL (hop limit)
  - Payload
- Nodes forward packets until TTL = 0
- Duplicate packets are dropped

-wiring here

---<img width="229" height="207" alt="Screenshot_20260116_083415" src="https://github.com/user-attachments/assets/661b8e8d-07c8-4e6c-8c73-c8ce5c2fd659" />


## 🔵 Bluetooth Control

Bluetooth Classic SPP lets you:
- Set node name
- Change LoRa parameters
- Send messages
- View node info
- Debug mesh behavior

Any Bluetooth Serial app works.

---

## ⚠️ Status

🚧 **EXPERIMENTAL**  
This firmware is under active development.

Do NOT rely on it for safety-critical use.

---

## 🛣️ Roadmap

- v0.1.x — stable mesh + CLI
- v0.2.x — Android app
- v0.3.x — better routing
- v1.0.0 — protocol freeze

---

## 📜 License

Apache License 2.0

By contributing, you agree that your contributions
may be relicensed by the project maintainer

You can:
- use it
- modify it
- sell hardware with it

Just don’t be a dick.
