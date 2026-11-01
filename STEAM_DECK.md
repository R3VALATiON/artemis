# Steam Deck personal testing preview — 0.3.1

**UNOFFICIAL PERSONAL DEVELOPMENT / TESTING BUILD — R3VALATiON.** Independent experimental fork. Not an official Artemis, Apollo or Valve release; no upstream endorsement is implied. This is not Valve Deck Verified certification.

Target checked on 2026-11-01: **SteamOS 3.9.5 stable**, announced by Valve on October 23, 2026 at https://www.steamdeck.com/en/news . Framework reference metadata comes from Valve's `holo-3.9`, `core-3.9` and `extra-3.9` package repositories. Repository availability is not proof that every package is installed on a particular Deck.

## Install and launch

Use the x86-64 portable `.tar.gz` for a FUSE-free launch:

1. In Desktop Mode, extract it to a permanent folder in your home directory, such as `~/Applications/Artemis-DualSense/`.
2. Run `./install-steamdeck.sh` inside the extracted folder. It registers a menu entry in your home directory and does not move the application. Keep that folder in place.
3. In Steam, add **Artemis DS Steam Deck Personal Test** as a non-Steam game. Its launch target should be `steamdeck-launch.sh` in that folder.
4. Leave Proton and forced compatibility tools off: this is a native Linux client.
5. Launch in Desktop Mode or switch to Game Mode. The wrapper selects Qt XCB + SDL X11 under gamescope; Desktop Mode can use native Wayland. The package includes both paths.

The AppImage is also included. Keep the supplied `steamdeck-launch.sh` beside it to use its FUSE-free extraction launcher. The portable archive avoids extracting the AppImage on every launch.

No `pacman`, root filesystem unlock, driver replacement, or SteamOS modifications are required by the application. It has a separate personal-testing settings identity and does not automatically replace itself with an upstream build.

## Controllers and streaming

Use the matching [Apollo Windows personal v0.2.0 preview](https://github.com/R3VALATiON/Apollo-Extended/releases/tag/v0.2.0-personal-preview.1). See `LINUX_DUALSENSE.md` for the native emulation toggle and Automatic / Always on / Off traffic controls.

The Deck's built-in controls retain ordinary desktop Artemis/SDL support. Native DualSense audio/haptics requires an external physical Sony DualSense or Edge; built-in Deck haptics are not a DualSense output device. One native DualSense per stream is supported, including a player slot after the built-in Deck controller.

Steam Input may expose a physical DualSense as a virtual Xbox controller. If that happens, adjust its Steam Input handling and reconnect so Artemis can see the physical Sony device. This interaction and built-in/external controller coexistence need on-device validation.

Valve's current steam-devices rules grant active-user access to USB and Bluetooth DualSense/Edge hidraw devices: https://github.com/ValveSoftware/steam-devices/blob/master/60-steam-input.rules . Use the Deck's existing rules first. If access is denied, the diagnostic prints the node; do not run Artemis as root or disable SteamOS read-only mode for this preview. USB is an alternative while Bluetooth permissions are investigated.

USB native audio requires exclusive access to the matching four-channel ALSA device. Stop other playback to the controller and reconnect if the audio card is busy. Native microphone forwarding and automatic controller-jack switching are not included.

## Framework checks

- Qt and Qt Quick/QML are bundled together, rather than mixed with SteamOS's newer Qt.
- SDL2 is bundled as SDL 2.32.10, avoiding dependence on SteamOS's SDL2-to-SDL3 compatibility layer.
- FFmpeg, Opus and OpenSSL libraries use the bundled versions and matching ABI majors.
- The SteamOS graphics/audio platform stack supplies Mesa, OpenGL/EGL/Vulkan, libdrm, GBM, libva, ALSA and PipeWire. These are not replaced with Ubuntu driver libraries.
- The launcher uses SteamOS's `/usr/lib/dri` for VA-API drivers when present.
- The release workflow audits all packaged ELF dependencies and glibc requirements, verifies Qt display/QML plugins, and smoke-tests the CLI and UI under X11 and a nested Wayland compositor at 1280×800.
- `COMPATIBILITY_AUDIT.json` records the exact packaged dependency checks. `STEAMOS_PACKAGE_REFERENCE.json` records the official repository reference versions. These are checks of software packaging/ABI, not a booted SteamOS installation test.

Run `./check-steamdeck.sh` inside the extracted portable folder for a read-only check on your actual Deck. It prints the OS/session, checks packaged ELF dependency loading, and reports DualSense node access. It does not submit haptic output or alter controller settings.

## Remaining hardware checks

A physical Deck is still required to verify gamescope launch/focus, built-in and external controller behavior, USB/Bluetooth native haptics, adaptive triggers, speaker audio, VA-API decode, latency adaptation, suspend/resume and reconnect behavior. The client clears native playback and renegotiates its lease after a long heartbeat gap; successful resume still depends on device/network recovery.

LCD/OLED-specific display rates, HDR, decoder performance and battery use are not certified by headless checks. Select resolution/refresh/codec appropriate for the actual Deck and host. A successful dependency audit does not establish every feature works on that hardware.
