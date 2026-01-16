# OpenMesh – Developer Notes

This file contains internal notes, design decisions, hacks, and reminders.
Not user documentation. Not API reference.
If you are reading this, you are either a maintainer or very curious.

---

## 🔧 Design Philosophy

- Cheap hardware > perfect performance
- Arduino-first, no RTOS dependency
- Bluetooth Classic > BLE (simpler, debuggable)
- LoRa CSS only (no FSK/OOK for now)
- OLED must work without phone
- Phone app is optional, not required

This is **not** Meshtastic.
This is OpenMesh: smaller, simpler, DIY-first.

---

## 🧠 Why Things Are Done This Way

### Why no GPS
- Cost
- Power
- Most DIY users don’t need it
- Node identity is enough

### Why Bluetooth Classic (SPP)
- Works with generic terminal apps
- Easier debugging
- Android friendly
- No BLE pain

### Why fixed AES key (for now)
- Simplicity
- Key management is hard
- Will be replaced later with pairing / key exchange

---

## 📡 LoRa Decisions

- Modulation: **LoRa CSS only**
- No FHSS (CSS already robust)
- Narrow bandwidth preferred
- Slow + long range > fast + unstable

Default target:
- SF12
- 62.5 kHz BW
- Max legal TX power

---

## 🔁 Mesh Logic Notes

- TTL-based forwarding
- No routing tables yet
- Anti-loop via recent msg ID cache
- Broadcast floods are acceptable at small scale

Known limitations:
- Can stall under heavy traffic
- No congestion control (yet)
- No ack / retry system

---

## ⚠️ Known Dumb Stuff (TODO FIX)

- Message length fixed to 16 bytes (AES block)
- No message fragmentation
- OLED UI blocks during TX sometimes
- Hardcoded pins
- No board profiles

Yes, we know.
No, it’s not a bug, it’s a roadmap.

---

## 🛠 Planned v0.2+ Ideas

- Fragmented messages
- Board profiles (pins.h)
- Per-node AES keys
- Optional BLE mode
- Better OLED layout
- Config save/load via BT
- Android app v1

---

## 🧪 Debug Notes

- If messages “hang”: check TX power + BW
- If OLED freezes: LoRa TX blocking
- If nothing works: check wiring (always)

---

## 💀 Rules for Contributors

- Do NOT add complexity without reason
- Do NOT break cheap hardware support
- Do NOT assume everyone has fancy boards
- If you change protocol, document it

---

## 🧃 Maintainer Notes

- This project was built fast, not perfect
- Stability > features
- DIY users come first

If this ever becomes popular:
- resist bloat
- resist vendor lock-in
- resist “enterprise” thinking

---

## 🐸 Final Notes

Yes, some code is ugly.
Yes, it works anyway.

If you can make it better without making it heavier:
PR welcome.