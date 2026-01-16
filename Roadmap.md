# OpenMesh Roadmap 🕸️

This is a roadmap, not a promise.
Things may change.
Bugs will appear.
Coffee will be consumed.

OpenMesh exists because:
- Expensive boards suck
- Closed ecosystems suck
- Overengineering sucks

---

## ✅ v0.1 – It Works (Mostly)

Status: **Pre-release, somehow alive**

### What we have
- ESP32 + SX127x support
- LoRa CSS (the good stuff)
- Mesh forwarding (TTL-based, no magic)
- Anti-loop protection (so it doesn’t scream forever)
- Broadcast & directed messages
- AES-256 encryption (don’t leak the key, bro)
- Bluetooth Classic control (SPP like it’s 2012)
- OLED UI for sending messages without a phone
- Button-driven UI (one button, many regrets)
- Auto-generated node names
- Passive node discovery (no handshakes, just vibes)

### Known pain points
- Messages are short (say it faster)
- No delivery confirmation (trust issues)
- Blocking TX (yes, it freezes sometimes)
- Pins are hardcoded (future-you problem)
- No routing logic (YOLO flood)

---

## 🔧 v0.2 – Make It Not Annoying

Goal: **Stability before flexing**

### Firmware
- Non-blocking LoRa TX (OLED no longer stuck on “sending…”)
- Message queueing (patience simulator removed)
- OLED UI cleanup (so text actually fits)
- Node name config via Bluetooth
- Save LoRa settings to flash
- Board pin profiles (`pins.h`) so people stop yelling
- Cleaner code organization (less chaos)

### Mesh
- Better duplicate detection
- Tunable relay delay (less packet screaming)
- TTL sanity improvements

---

## 🔐 v0.3 – Stop Trusting Everyone

Goal: **Security that isn’t fake**

### Firmware
- Variable-length messages
- Fragmentation & reassembly (long texts finally allowed)
- Per-node keys (optional)
- Optional pairing-based key exchange
- Protocol version checks (no mystery packets)

### Protocol
- OpenMesh Protocol v1
- Message types
- Flags that actually mean something
- Backward compatibility (hopefully)

---

## 📱 v0.4 – Phone App (No GPS, Chill)

Goal: **Actually usable ecosystem**

### Android App
- Bluetooth Classic connection
- Node list
- Chat UI
- LoRa config sliders
- Mesh info & hop count
- No GPS tracking
- No cloud
- No account
- No ads
- No bullshit

### Firmware
- Structured BT command protocol
- JSON responses (apps love JSON)
- Optional headless mode (no OLED)

---

## 🌐 v0.5 – Bigger Mesh, Less Noise

Goal: **Scaling without melting the air**

### Mesh
- Reduced broadcast flooding
- Smarter relay logic
- Optional store-and-forward
- Congestion handling (radio breathing room)

### Firmware
- Power-saving modes
- Sleep support
- Battery awareness
- Less heat, less pain

---

## 🚀 v1.0 – Stable DIY Mesh™

Goal: **It just works**

- Stable protocol
- Documented firmware
- Usable Android app
- Multiple board profiles
- Predictable behavior
- No surprise features

Still not enterprise.
Still not commercial.
Still DIY as hell.

---

## ❌ Things We Are Not Doing

OpenMesh will NOT:
- Require GPS
- Require internet
- Require accounts
- Require cloud services
- Lock hardware
- Hide protocols
- Turn into a crypto project
- Add AI (bro please no)

---

## 🧠 Final Notes

If a feature:
- Makes hardware expensive ❌
- Makes code unreadable ❌
- Makes users confused ❌

It probably won’t be added.

Cheap.
Simple.
Open.