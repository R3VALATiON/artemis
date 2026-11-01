# Artemis Steam Deck 0.3.1 Preview 1

**UNOFFICIAL PERSONAL DEVELOPMENT / TESTING BUILD — R3VALATiON.** Independent experimental fork; not an official upstream release. Physical controller behavior is unverified.

Steam Deck revision of the Linux native DualSense preview, retaining the desktop V2 and Linux 0.3.0 changes. App version: `0.6.7-personal-dev.0.3.1`; release tag: `v0.3.1-steamdeck-personal-preview.1`.

- Native DualSense / DualShock 4 / Xbox / Automatic emulation selection for physical DualSense devices, with a native-emulation master toggle.
- Automatic / Always on / Off native feedback controls in Settings and in the in-stream Quick Menu.
- The same ≥12 Mbps configured video target, 20-second stabilization, 150/120 ms latency hysteresis and poor-connection handling as the Android preview.
- Encrypted Apollo policy negotiation, acknowledgement filtering and renewable six-second host traffic leases, using the matching Windows personal host v0.2.0 preview.
- Native four-channel USB ALSA output, plus experimental Bluetooth native waveform transport and Opus speaker frames. Standard SDL inputs/effects remain available independently.
- A bounded reorder window, nonblocking output, route validation and a controller-index bounds fix.
- Separate personal-testing settings identity, disabled upstream automatic updates, protocol tests, build provenance and checksums.

## Steam Deck revision

- Target SteamOS 3.9.5 stable; bundle Qt 6.8.3 and SDL 2.32.10 together with compatible media libraries.
- FUSE-free portable launch, user-menu installer, Game Mode XCB/SDL X11 launcher and native Wayland display plugins.
- Preserve the host Mesa/libdrm/GBM/libva/ALSA/PipeWire stack and select the SteamOS VA-API driver directory.
- Audit every packaged ELF dependency, architecture and glibc requirement; smoke-test X11 and Wayland UI startup at 1280×800.
- Provide a read-only on-Deck preflight and a versioned official SteamOS repository reference.
- Include suspend time in feedback lease deadlines; silence and renegotiate after a long heartbeat gap.
- Identify a native controller by its physical HID path as well as serial; refuse ambiguous output routes.
- Skip NVIDIA CUVID decoder probing when its runtime is absent, avoiding an inherited Linux startup crash.

Read STEAM_DECK.md and the attached compatibility audit. The software checks are not a booted SteamOS or physical Deck test, and do not constitute Valve certification.

## Downloads

- x86-64 AppImage, with Qt and media dependencies bundled.
- Portable x86-64 `.tar.gz` of the same application for systems without FUSE.
- Linux setup/limitations guide, optional Bluetooth udev rule, build provenance and SHA256SUMS.

Read `LINUX_DUALSENSE.md` before hardware testing. One native controller per stream. Automatic headset-jack routing and controller microphone forwarding from Android are not included. New network and physical-controller paths remain experimental; headless CI cannot validate them.

This is client-side Linux support. Continue using the matched Apollo Windows personal host. Original project credits and licenses are retained; all changes stay in R3VALATiON's fork.
