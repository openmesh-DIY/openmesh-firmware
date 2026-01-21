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