# OpenMesh FAQ 🔥

## Why OpenMesh?
Because Meshtastic is cool  
but my wallet said **absolutely not**.

Also because:
- DIY hardware deserves respect
- Not everyone wants a $50 dev board
- I like controlling my own stuff
- I don’t enjoy begging apps to work offline

---

## Is this a Meshtastic clone?
Kinda.

Let’s be honest:
Meshtastic was a **huge inspiration**.

But:
- Meshtastic is a full ecosystem
- OpenMesh is just a **firmware**
- No cloud
- No GPS obsession
- No forced app dependency

Think:
- Meshtastic = Swiss Army Knife
- OpenMesh = Sharp rock made from spite
- One is polished, the other works because it has to

Inspired? Yes.  
Cloned? No.  
Emotionally motivated? Absolutely.

---

## Does this support GPS?
No.

If you need satellites to say “hello”,
you might be overengineering your personality.

---

## Does this support internet?
Also no.

If your mesh needs WiFi to function,
that’s not a mesh,
that’s Stockholm syndrome.

---

## Is this secure?
Secure enough to talk.
Not secure enough to overthrow anything.

AES-256 is used, but:
- Keys are shared manually
- No fancy crypto handshakes
- No automatic key rotation
- No illusion that this is Fort Knox

This is radio.
Not a spy movie.

---

## Why AES-GCM and not CBC?
Because:
- GCM handles authentication for you
- IVs don’t desync and ruin your night
- Less “3am caffeine-fueled debugging”
- Less chance of cryptographic oopsies

Think:
- GCM = instant noodles, always works
- CBC = handmade noodles, tasty but one mistake and everything explodes

My heart rate prefers GCM.

---

## What are CTR and ECB?
- **ECB**: don’t use it  
  (If you use ECB, this firmware will judge you silently)
- **CTR**: fast but fragile if you mess up counters

GCM exists so we don’t have to learn these lessons the hard way.

---

## Why Arduino and not ESP-IDF?
Because this project is for **humans**.

Arduino means:
- Faster onboarding
- Less pain
- More contributors
- Less gatekeeping

ESP-IDF enjoyers are welcome,
but please don’t rewrite the entire codebase out of boredom.

---

## Will this replace Meshtastic?
No.

Meshtastic replaces OpenMesh
in situations where money exists.

OpenMesh replaces:
- Your suffering
- Your dependence on vendors
- Your expectation that things should “just work”

---

## What hardware does this support?
Currently:
- ESP32 (WROOM32 recommended)
- SX1276 / SX1278
- SSD1306 OLED

More boards *might* come later.
Depends on:
- Motivation
- Free time
- Coffee levels

---

## Why is the UI so simple?
Because:
- OLEDs are tiny
- One button is enough
- Simple UIs don’t crash
- Minimalism saves sanity

Also it looks kinda cool.

---

## Why no delivery confirmation?
Because radio.

If you want guaranteed delivery:
- Use the internet
- Or shout louder
- Or accept reality

---

## Is this beginner-friendly?
Yes.

If you can:
- Flash an ESP32
- Wire a few pins
- Read basic code

Congrats, you qualify.

---

## Can I sell devices running OpenMesh?
Depends on the license.

Read the LICENSE file.
No seriously.
Read it.
This is not a joke section.

---

## Can I contribute?
Yes.

Rules:
- Keep it simple
- Document your changes
- Don’t break DIY users
- Don’t remove jokes
- Don’t add “enterprise features”

---

## Why is the maintainer like this?
Unknown.

Possible causes:
- Lack of sleep
- Too much caffeine
- Debugging radios at 3am
- Breadboards with trust issues

---

## Is this good for camping?
Yes.

Perfect for:
- Camping
- Off-grid chats
- Yelling “WHERE ARE YOU?” digitally

Just make sure your phone battery is charged  
to watch po—  
I mean…  
to **communicate**.

---

## Final Answer
OpenMesh exists because someone needed it.

That someone was broke.

And tired.

And done with proprietary nonsense.