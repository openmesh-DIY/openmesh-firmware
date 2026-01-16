 OpenMesh Architecture

> "If you think this should be rewritten in Rust, this document is not for you."



This file explains how OpenMesh works, why it works this way, and why some of your ideas were rejected before you even typed them.

Read this before opening issues, PRs, or philosophical debates.


---

🎯 Design Goals (aka: constraints from reality)

OpenMesh was built with:

Cheap hardware

Low power radios

Limited RAM

Limited flash

One tired developer


Priorities:

✅ Works offline

✅ Predictable behavior

✅ Easy to debug

✅ Hard to accidentally brick


Not priorities:

❌ Academic perfection

❌ Nation‑state threat models

❌ Infinite scalability

❌ Impressing Hacker News



---

 Mesh Model (Simple. On purpose.)

OpenMesh uses a flood-based mesh with hop limiting.

Why flooding?

Because:

Routing tables lie

Dynamic topologies are hell

Memory is small

Radios are dumb


Flooding is:

Stupid

Reliable

Predictable

Honest


If you are about to suggest AODV / DSR / OLSR / BGP / "my thesis": Stop. This is a microcontroller, not a data center.


---

🔁 Packet Flow

1. Node receives packet


2. If packet ID seen before → drop


3. If hop count exceeded → drop


4. Decrypt payload


5. Process payload


6. Re-broadcast packet



Yes, every node repeats packets.

No, this is not "inefficient".

This is how radios survive reality.


---

🔐 Cryptography (Read slowly)

AES‑256

Shared static key

Payload encryption only


Why static keys?

Because:

Key exchange costs bytes

Bytes cost airtime

Airtime costs power

Power costs range


Also because:

Users can understand it

Debugging is sane

Failure modes are obvious


If your response is:

> "But perfect forward secrecy—"



Congratulations, you just volunteered to design, audit, implement, document, and support it.


---

🚫 Things This Does NOT Try To Solve

Trust

Identity

Anonymity

Physical capture

Compromised nodes

Malicious relays


If you deploy OpenMesh assuming those are solved: That is a you problem, not a firmware bug.


---

📡 Radio Layer

Dumb radios

No magic

No QoS

No retries at PHY


Reliability is handled above the radio.

If you want guaranteed delivery: Use a wire.


---

🧵 Concurrency Model

Single main loop

No RTOS magic

No background threads doing spooky things


Why? Because race conditions are harder to debug than slow code.


---

🧪 Error Handling Philosophy

Drop bad packets

Log if possible

Move on


The network should survive bad nodes. If your node crashes the mesh: That is a bug.


---

🧾 On Contributions (Read This Twice)

Good PRs:

Fix real bugs

Improve clarity

Reduce complexity

Match existing style


Bad PRs:

"I refactored everything"

"I reformatted all files"

"I added a framework"

"I rewrote this in Rust"


If your PR:

Adds abstraction layers

Adds macros for no reason

Adds dependencies


It will be closed. Not discussed. Closed.


---

🧠 A Note to Smart People

Yes, this could be "better".

It could also:

Use more RAM

Use more flash

Be harder to debug

Fail in worse ways


OpenMesh chooses boring correctness over cleverness.

Linus said it best (paraphrased):

> "If your code is clever, it is probably wrong."




---

🛑 Final Warning

If you are offended by this document:

You are not the target user

You will not enjoy contributing

That is okay


If you understood this document and still want to help: Welcome.

You’re one of the few.