# OpenMesh Firmware

OpenMesh Firmware – Technical Advisory
Dear Community Members and Users,
This notice is issued to provide a technical status update regarding the current OpenMesh firmware release.
Recent testing has identified a known limitation affecting acknowledgement (ACK) handling within the mesh layer. Under certain network conditions, ACK packets may be relayed or replayed in a manner that can result in duplicate confirmations or inconsistent acknowledgment behavior. While core message delivery and mesh forwarding remain functional, acknowledgment signaling may not always reflect ideal or deterministic behavior.
At this time:
Message transmission and mesh routing are operational
Payload integrity and encryption are not affected
ACK behavior may exhibit replay or duplication artifacts
This firmware is not classified as broken, but it should be considered beta-stable and subject to refinement. Users who require strict delivery confirmation semantics or mission-critical guarantees should be aware of this limitation.
A revised firmware addressing this issue is planned and will be released when testing and validation are complete. Development is ongoing, and additional logging and diagnostics are being used to guide improvements.
Known Issues
ACK replay / duplication
Acknowledgement packets may be re-forwarded by relay nodes, leading to repeated ACKs being received by the sender.
Non-deterministic ACK paths
In multi-hop scenarios, ACKs may return via different routes than the original message, affecting perceived reliability metrics (e.g. RSSI correlation).
ACK timing inconsistency
Delays may occur under high relay density or low data rate configurations (e.g. high SF), which can trigger unnecessary retries.
ACK reliability decreases with hop count
ACK accuracy is best in 1–2 hop topologies and may degrade in larger or denser meshes.
These issues do not affect payload encryption, message forwarding, or node discovery, and are limited to acknowledgement signaling only.
Usage Guidance
This firmware is intended for:
DIY experimentation
Off-grid mesh communication testing
Non-critical field use
It is not a certified emergency communication system. Users deploying this firmware in outdoor or remote environments do so at their own discretion and should maintain appropriate fallback communication methods.
Further updates will be provided alongside future releases.
— OpenMesh Project

<!--
This project exists because "cheap" shouldn't mean "locked down".
-->

⚠️ **Warning**  
This firmware may contain bugs, bad ideas, and decisions made after midnight.  
If it works for you: congrats.  
If it doesn’t: welcome to DIY.

Bluetooth messages are **NOT end-to-end encrypted beyond the device**.  
That is intentional. That is documented. That is on you.

---

## What This Is

OpenMesh is a **cheap, DIY, Arduino-first LoRa mesh firmware**  
for **ESP32 + SX127x radios**.

It exists so **normal people** can build mesh communication without:

- expensive boards
- vendor lock-in
- forced GPS
- forced cloud
- forced apps
- corporate vibes

> OpenMesh is not perfect.  
> Neither are you.  
> That’s fine.

⚠️ **Important:**  
The default AES key is **PUBLIC** and intended for testing only.  
If you deploy without changing it, you deployed a demo.

---

## Philosophy

- If you want polished UX → buy a product  
- If you want control → build it yourself  
- If you want both → pick one and cope  

This firmware favors:
- transparency
- simplicity
- things you can actually debug

---

## 🔥 Core Principles

- Cheap hardware
- Arduino IDE friendly
- Full user control
- Bluetooth Classic (SPP) configuration
- Works without internet
- Open protocol
- No forced dependencies

---

## ✨ Features

- ESP32 (WROOM32 recommended)
- SX1276 / SX1278 LoRa (CSS)
- Broadcast & directed messages
- Mesh routing with TTL (hop count)
- Duplicate packet suppression
- Node ID + human-readable node name
- Bluetooth Classic (SPP) CLI
- OLED UI (SSD1306)
- AES-256-GCM encrypted payloads (optional)
- No GPS
- No vendor servers
- No cloud
- No account
- No excuses

---

## 📱 Phone Requirement (v0.1.2+)

**Phone is required (for now).**

- OLED messaging **was left** in v0.1.2
- OLED is currently UI + status only
- Messaging moved to Bluetooth for stability and sanity

Standalone messaging **will return** later  
when the firmware stops trying to fight physics.

If you’re upset:
- wait
- contribute
- or yell at RF noise instead

---

## 🧱 Hardware

### Minimum
- ESP32
- SX1276 / SX1278
- OLED SSD1306 (128x64)

### Supported Bands
- 433 MHz
- 868 MHz
- 915 MHz
(depends on lora mines only goes up to 525mhz)

> Breadboards work.  
> Dirty breadboards lie.  
> Proto PCB is peace.

---

## 📡 How It Works (Simple Version)

Every node has:
- A **Node ID**
- A **Node Name**

Each message contains:
- SRC (sender)
- DEST (receiver or broadcast)
- TTL (hop limit)
- Encrypted payload

Routing rules:
- Nodes forward packets until TTL = 0
- Duplicate packets are dropped
- Relays see metadata, not plaintext
- RF is still RF — don’t romanticize it

---

## 🔵 Bluetooth Control

Bluetooth Classic (SPP) is used.

You can:
- Send messages
- Change presets
- Set frequency
- View node info
- Debug mesh behavior
- Panic less

Any Bluetooth Serial app works.

Assume:
- If it’s paired, it’s trusted
- If it’s exposed, that’s your choice

---

## 🔐 Security (Short Version)

- AES-256-GCM for payload encryption
- Static shared mesh key
- Manual key management by design

Why GCM?
- Encryption + authentication in one shot
- No DIY MAC disasters
- No “decrypts but lies”
- Bad packets fail honestly

This is **practical security**, not cosplay.

If your threat model includes:
- nation states
- jail time
- or helicopters

Stop.  
Use something else.

---

## ⚠️ Status

🚧 **EXPERIMENTAL / PRE-STABLE**

Things may:
- change
- move
- disappear
- reappear better
- or get rewritten at 3 AM

Do NOT rely on this firmware for:
- safety
- emergencies
- life decisions
- impressing people

---

## 🛣️ Roadmap

- **v0.1.x** — stable mesh + CLI
- **v0.2.x** — Android app
- **v0.3.x** — better routing
- **v1.0.0** — protocol freeze, less chaos

Roadmaps are intentions, not promises.

---

## 📜 License

Apache License 2.0

You can:
- use it
- modify it
- sell hardware with it

By contributing, you agree your code may be relicensed.

Just don’t be a dick.

---