# OpenMesh Dev Notes 🧠🔥

These are internal notes.
If you are reading this:
- Welcome
- You are now partially responsible

This file exists so future contributors understand:
- Why things are the way they are
- Why some things are NOT the way they “should be”
- Why the maintainer occasionally sounds insane

---

## 🛠️ Design Philosophy

OpenMesh is built on 4 core principles:

1. Cheap hardware > fancy features  
2. Simple logic > “smart” magic  
3. Offline-first > cloud brainrot  
4. If it works, don’t touch it

If a feature:
- Requires expensive boards ❌
- Requires internet ❌
- Requires GPS ❌
- Requires an account ❌

It does NOT belong here.

---

## 🧱 Why This Code Looks Like This

Yes, the code is:
- Direct
- Flat
- Sometimes aggressive

That is intentional.

This firmware is designed so:
- A beginner can read it
- A hacker can modify it
- A broke student can afford it

If you want abstraction layers, dependency injection, and 500 files:
➡️ Wrong repo.

---

## 🧪 About Stability

This project is **experimental**.

That means:
- Things might break
- Messages might drop
- Nodes might reboot
- Your friend might say “bro it doesn’t work”

Congratulations.
That’s called *radio*.

---

## 🕸️ Mesh Logic Notes

- OpenMesh uses **TTL-based flooding**
- No routing tables
- No path discovery
- No global state

Why?

Because:
- Routing tables rot
- Dynamic topology is pain
- Flooding works and radios are slow anyway

Future versions *may* improve this, but only if it stays simple.

---

## 🔐 Security Reality Check

AES-256 is used.

This means:
- Casual eavesdroppers ❌
- Curious neighbors ❌
- Script kiddies ❌

This does NOT protect you from:
- Nation states
- Physical access
- Someone who steals your firmware + key
- Yourself posting the key on GitHub

If you need military-grade OPSEC:
➡️ Stop using Arduino.

---

## 📡 Bluetooth Notes

Bluetooth Classic (SPP) is used because:
- It’s simple
- It works everywhere
- Terminal apps exist
- No BLE pain

Yes, BLE is modern.
No, we don’t care (yet).

---

## 🧩 Easter Eggs 🥚

If you found comments like:
- “don’t mess this up”
- “I’m too lazy”
- “AIN’T MY FAULT”

Congratulations.
You found developer morale boosters.

Do not remove them.
They are load-bearing comments.

---

## 🚫 Things That Will Get Your PR Rejected

- Adding GPS “because Meshtastic has it”
- Adding cloud sync
- Adding login systems
- Adding AI (tbh idk why my friend added this)
- Rewriting everything for ESP-IDF “for cleanliness”
- Removing humor

Yes, removing humor is a valid rejection reason.

---

## ☕ Final Dev Rule

If you’re about to add a feature:
- Ask: does this help DIY users?
- Ask: does this keep things cheap?
- Ask: will this confuse beginners?

If the answer is “no”:
➡️ Don’t add it.

---

If this project blows up:
- Nice
If it doesn’t:
- Still nice

At least it exists.