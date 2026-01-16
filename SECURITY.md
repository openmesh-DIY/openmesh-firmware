# Security Policy

## Supported Versions

OpenMesh is an **experimental, DIY-first project**.

Only the **latest release** is considered supported.

Older versions may:
- contain known vulnerabilities
- lack fixes
- reflect bad decisions from the past

| Version | Supported |
|--------|-----------|
| Latest | ✅ |
| Older | ❌ (I guess so...) |

---

## Threat Model (Please Read Before Getting Creative)

OpenMesh is designed for:
- Hobbyists
- DIY users
- Offline communication
- Low-cost hardware
- People who enjoy understanding their tools

It is **NOT** designed for:
- Military operations
- High-risk activism
- Life-or-death coordination
- Nation-state threat models
- Anyone whose plan includes the phrase “but it should be secure enough”

If your safety depends on this firmware:

**Stop.  
Re-evaluate.  
Breathe.**

Then choose a system designed for that level of risk.

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
- Key management is manual by design

This means:
- All nodes must use the same key
- If the key leaks, the mesh is compromised
- There is no automatic key rotation
- This is not a blockchain, and that’s intentional

Security here is **practical**, not ceremonial.

---

## Bluetooth Security

Bluetooth Classic (SPP) is used.

Assume:
- Anyone paired can send commands
- Anyone with access can read messages
- Pairing is a trust decision, not a suggestion

If you expose your node to untrusted devices:
that is not a vulnerability,
that is a life choice.

---

## Responsible Disclosure

If you find a security issue:

1. **Do NOT open a public issue**
2. Contact the maintainer privately
3. Provide:
   - Firmware version
   - Hardware used
   - Steps to reproduce
   - Impact assessment (be honest)

Please do not:
- Drop zero-days in GitHub issues
- Post exploits on social media
- Start sentences with “I didn’t test this but…”

---

## Scope Clarification

The following are **not** considered vulnerabilities:
- RF being interceptable
- Mesh relays seeing metadata
- Someone flashing their own firmware
- You losing your device
- You sharing your AES key in Discord

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