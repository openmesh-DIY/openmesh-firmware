# Security Policy 

## Supported Versions

OpenMesh is an **experimental DIY project**.

Only the **latest release** is considered supported.
Older versions may contain known vulnerabilities and are not guaranteed to be safe.

| Version | Supported |
|--------|-----------|
| Latest | ✅ |
| Older | ❌(idk) |

---

## Threat Model (Read This First)

OpenMesh is designed for:
- Hobbyists
- DIY users
- Offline communication
- Low-cost hardware

It is **NOT** designed for:
- Military operations
- High-risk activism
- Life-or-death coordination
- Nation-state threat models

If your safety depends on this firmware:
**Stop. Re-evaluate. Breathe.**

---

## What OpenMesh Protects Against

✔️ Casual eavesdropping  
✔️ Curious neighbors  
✔️ Script kiddies  
✔️ Accidental plaintext leaks  

---

## What OpenMesh Does NOT Protect Against

❌ Physical access to your device  
❌ Firmware extraction  
❌ Shared or leaked AES keys  
❌ Malicious relays  
❌ Radio jamming  
❌ Advanced attackers  
❌ Bad decisions  

---

## Encryption Notes

- AES-256 is used for payload encryption
- Keys are static and shared between nodes
- Key management is manual

This means:
- All nodes must use the same key
- If the key leaks, the mesh is compromised
- There is no automatic key rotation

This is intentional to keep the system simple.

---

## Bluetooth Security

Bluetooth Classic (SPP) is used.

Assume:
- Anyone paired can send commands
- Anyone with access can read messages

Do not expose your node to untrusted devices.

---

## Responsible Disclosure

If you find a security issue:

1. **Do NOT open a public issue**
2. Contact the maintainer privately
3. Provide:
   - Firmware version
   - Hardware used
   - Steps to reproduce
   - Impact assessment

Please do not:
- Drop zero-days in issues
- Post exploits on social media
- “Just fix it yourself” without reporting

---

## Final Warning

OpenMesh gives you **freedom**.

Freedom means:
- Control
- Responsibility
- Consequences

Use wisely.