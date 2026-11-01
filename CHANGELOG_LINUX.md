# Artemis Linux 0.3.0 Preview 1

**UNOFFICIAL PERSONAL DEVELOPMENT / TESTING BUILD — R3VALATiON.** Independent experimental fork; not an official upstream release. Physical controller behavior is unverified.

Initial Linux port of the personal DualSense changes, based on the existing desktop V2 branch. App version: `0.6.7-personal-dev.0.3.0`; release tag: `v0.3.0-linux-personal-preview.1`.

- Native DualSense / DualShock 4 / Xbox / Automatic emulation selection for physical DualSense devices, with a native-emulation master toggle.
- Automatic / Always on / Off native feedback controls in Settings and in the in-stream Quick Menu.
- The same ≥12 Mbps configured video target, 20-second stabilization, 150/120 ms latency hysteresis and poor-connection handling as the Android preview.
- Encrypted Apollo policy negotiation, acknowledgement filtering and renewable six-second host traffic leases, using the matching Windows personal host v0.2.0 preview.
- Native four-channel USB ALSA output, plus experimental Bluetooth native waveform transport and Opus speaker frames. Standard SDL inputs/effects remain available independently.
- A bounded reorder window, nonblocking output, route validation and a controller-index bounds fix.
- Separate personal-testing settings identity, disabled upstream automatic updates, protocol tests, build provenance and checksums.

## Downloads

- x86-64 AppImage, with Qt and media dependencies bundled.
- Portable x86-64 `.tar.gz` of the same application for systems without FUSE.
- Linux setup/limitations guide, optional Bluetooth udev rule, build provenance and SHA256SUMS.

Read `LINUX_DUALSENSE.md` before hardware testing. One native controller per stream. Automatic headset-jack routing and controller microphone forwarding from Android are not included. New network and physical-controller paths remain experimental; headless CI cannot validate them.

This is client-side Linux support. Continue using the matched Apollo Windows personal host. Original project credits and licenses are retained; all changes stay in R3VALATiON's fork.
