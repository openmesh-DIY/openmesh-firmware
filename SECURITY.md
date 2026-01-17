# Security Policy

## Supported Versions

OpenMesh is an **experimental, DIY-first project**.

Only the **latest release** is considered supported.  
Everything else is archaeology.

Older versions may:
- contain known vulnerabilities
- lack fixes
- reflect bad ideas that made sense at 3 AM

| Version | Supported |
|--------|-----------|
| Latest | ✅ |
| Older | ❌ (I guess so…) |

---

## Threat Model (Read This Before Doing Something Stupid)

OpenMesh is designed for:
- Hobbyists
- DIY users
- Offline communication
- Low-cost hardware
- People who like knowing *how* things work
- Camping (yes, actually useful outside your bedroom)

It is **NOT** designed for:
- Military operations
- High-risk activism
- Life-or-death coordination
- Nation-state threat models
- Anyone whose plan includes  
  > “but it should be secure enough”

(Still good for camping though.  
Just make sure your phone battery is charged —  
for communication.  
Obviously..)

If your safety depends on this firmware:

**Stop.  
Re-evaluate.  
Breathe.**

Then pick a system that was built for that level of risk.

---

## What OpenMesh Protects Against

✔️ Casual eavesdropping  
✔️ Curious neighbors  
✔️ Script kiddies  
✔️ Accidental plaintext leaks  
✔️ “I opened Wireshark and now I’m a hacker” energy  

---

## What OpenMesh Does NOT Protect Against

❌ Physical access to your device  
❌ Firmware extraction  
❌ Shared or leaked AES keys  
❌ Malicious relay nodes  
❌ Radio jamming  
❌ Advanced attackers  
❌ Anyone with patience, money, or a spectrum analyzer  
❌ Bad decisions  

---

## Encryption Notes

- AES-256 is used for payload encryption
- Keys are static and shared between nodes
- Key management is **manual by design**

This means:
- All nodes must use the same key
- If the key leaks, the mesh is compromised
- There is no automatic key rotation
- This is not a blockchain, and that’s intentional

Security here is **practical**, not ceremonial.

If you want:
- key servers
- automatic rotation
- zero trust
- magic

You are in the wrong repo.

---

## Bluetooth Security

Bluetooth Classic (SPP) is used.

Assume:
- Anyone paired can send commands
- Anyone paired can read messages
- Pairing is a **trust decision**, not a suggestion

If you expose your node to untrusted devices:
that is not a vulnerability,
that is a lifestyle choice.

---

## Responsible Disclosure

If you find a security issue:

1. **Do NOT open a public issue**
2. Contact the maintainer privately
3. Include:
   - Firmware version
   - Hardware used
   - Steps to reproduce
   - Impact assessment (be honest)

Please do not:
- Drop zero-days in GitHub issues
- Post exploits on social media
- Start sentences with  
  > “I didn’t test this but…”

---

## Scope Clarification

The following are **NOT** considered vulnerabilities:

- RF being interceptable
- Mesh relays seeing metadata
- Someone flashing their own firmware
- You losing your device
- You pasting your AES key into Discord

---

## Final Warning

OpenMesh gives you **freedom**.

Freedom includes:
- Control
- Responsibility
- Consequences

If something breaks because you ignored this file,
the firmware worked exactly as intended.

Use wisely.