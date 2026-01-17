# OpenMesh Roadmap

This roadmap describes **intent**, not promises.

Dates are imaginary.  
Features arrive when:
- the hardware agrees
- the RF gods allow it
- and the maintainer isn’t questioning life choices at 3 AM

---

## Phase 0 — It Boots (✅ Somehow)

- [x] Device turns on
- [x] OLED lights up instead of staying dead like my motivation
- [x] LoRa transmits *something*
- [x] AES-GCM encrypts without exploding
- [x] Bluetooth works without pairing to the wrong device in the room

Status:  
**Barely stable, but confident enough to show friends.**

---

## Phase 1 — Usable Without Crying (Current)

- [x] Encrypted LoRa messaging
- [x] Broadcast + node IDs
- [x] RSSI / SNR display
- [x] Modem presets (speed > range, because patience is overrated)
- [x] Boot-time hardware test that fails fast and judges you silently
- [x] Bluetooth CLI for people who hate buttons

Known issues:
- RF noise exists
- Breadboards lie
- USB power is emotional
- Antennas are either amazing or pure fiction

---

## Phase 2 — Mesh, But Not “Blockchain Bro” Mesh

Planned:
- [ ] Smarter relay logic (less echo, more signal)
- [ ] TTL handling that doesn’t feel drunk
- [ ] Neighbor table cleanup (ghost nodes begone)
- [ ] Message deduplication that actually works
- [ ] Optional relay disable (because sometimes you don’t want to help)

Non-goals:
- Trustless consensus
- Tokenomics
- Whitepapers
- Anything involving the word “immutable”

---

## Phase 3 — UX Without Selling Your Soul

Planned:
- [ ] Cleaner UI
- [ ] Less menu diving
- [ ] More “this makes sense”
- [ ] Fewer “why is it doing that?” moments

Maybe:
- [ ] Config profiles
- [ ] Saved presets
- [ ] Settings that persist across reboots (wild concept)

Still no:
- Touchscreens
- Mobile apps
- Cloud dashboards
- Subscriptions (absolutely not)

---

## Phase 4 — Security Improvements (Realistic Edition)

Planned:
- [ ] Better replay resistance
- [ ] Cleaner nonce handling
- [ ] Optional key separation
- [ ] Fewer foot-guns, same freedom

Not planned:
- Automatic key exchange
- PKI
- Certificates
- Anything that turns this into a PhD thesis

Security goal:
**Good enough for reality, not good enough for a spy movie.**

---

## Phase 5 — “Why Does This Exist?” Features

Possible:
- [ ] Noise floor scanner
- [ ] RF debugging modes
- [ ] Passive monitor mode
- [ ] “Is the air dead or am I?” indicators

Definitely not:
- AI-powered routing
- Machine learning
- Neural meshes
- Any feature that requires explaining to investors

---

## Long-Term Philosophy

OpenMesh will:
- Favor simplicity over cleverness
- Prefer honesty over magic
- Fail loudly instead of lying quietly

OpenMesh will not:
- Babysit you
- Protect you from yourself
- Pretend RF is deterministic
- Make bad antennas good

---

## Final Notes

If this roadmap feels vague:
- Good
- That means it’s honest

If something isn’t here:
- Either it’s unnecessary
- Or it sounded cool at 2 AM and got deleted at 2:05

Roadmaps change.  
Physics doesn’t.

Proceed accordingly.