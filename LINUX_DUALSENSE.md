# Artemis Linux — Unofficial Personal Development / Testing Preview

Maintained by R3VALATiON. Independent experimental fork, not an official release of Artemis, Apollo, their Extended variants, Moonlight, or Sunshine. No upstream endorsement is implied. Physical DualSense and end-to-end game behavior have not been tested.

## Run

Steam Deck-specific launch and compatibility details are in [STEAM_DECK.md](STEAM_DECK.md).

Linux x86-64, built on Ubuntu 22.04 (glibc 2.35 baseline). Suitable for testing on current x86-64 Linux desktops and Steam Deck desktop mode; SteamOS/Game Mode, Nobara, and other distributions remain hardware-unverified. ARM64 is not included.

For the AppImage:

```sh
chmod +x Artemis-SteamDeck-Personal-Testing-0.3.1-preview.1-x86_64.AppImage
./Artemis-SteamDeck-Personal-Testing-0.3.1-preview.1-x86_64.AppImage
```

If FUSE is unavailable, use `./Artemis-SteamDeck-Personal-Testing-0.3.1-preview.1-x86_64.AppImage --appimage-extract-and-run`, or extract the portable `.tar.gz` and run `./AppRun` inside its folder. The archive contains the same application and bundled dependencies.

The preview uses a separate settings identity, **Artemis DS Personal Test**, and does not automatically update from upstream. No root access is needed to launch it.

## Matching Windows host

Use [Apollo personal preview v0.2.0-personal-preview.1](https://github.com/R3VALATiON/Apollo-Extended/releases/tag/v0.2.0-personal-preview.1). Host driver/runtime requirements still apply. DSX is not required by this transport. Native game support is game-dependent; the new traffic lease requires this matching host protocol.

In Artemis Settings → Gamepad Settings:

- Enable **Native DualSense emulation**, and use Automatic or DualSense controller selection. Disabling native emulation maps Automatic/DualSense selections to DualShock 4; explicit Xbox is preserved.
- Select Automatic / Always on / Off for controller audio and HD haptics. The Quick Menu can cycle this setting during streaming.
- Automatic needs at least 12 Mbps configured video bitrate and 20 stable seconds. Poor connectivity or RTT ≥150 ms pauses traffic; recovery requires RTT <120 ms and 20 stable seconds. The configured bitrate is a policy threshold, not a measured spare-bandwidth estimate.
- Identity changes need a reconnect and may need a game restart. Connection adaptation changes PCM traffic without changing controller identity. Ordinary inputs, rumble, adaptive triggers, LEDs and motion remain separate.

One physical native DualSense or DualSense Edge is supported per stream. Other controllers retain ordinary SDL handling. Avoid remapping the client controller to a virtual Xbox device in Steam Input if you want native DualSense detection.

## USB

Connect the controller using a data-capable USB cable. This preview identifies the Sony ALSA card and opens its exact four-channel, 48 kHz S16_LE hardware output: speaker/jack channels 1/2, haptics channels 3/4. It never sends haptic PCM to your desktop's default audio device.

The controller ALSA device must be available for exclusive playback. If PipeWire, PulseAudio or another app has it open, stop playback to the controller, let it become idle, and reconnect the stream. Check the log for `DualSense native USB four-channel ALSA route ready` or an audio-device error. Multiple ambiguous matching audio devices are deliberately not selected. Automatic headset-jack routing and controller microphone forwarding from the Android build are not ported in this preview.

## Bluetooth

Pair the physical DualSense normally in Linux. Inputs and ordinary effects use SDL; native haptics/speaker packets use its matching `hidraw` node. Native waveforms are low-pass filtered and converted to the controller's 3 kHz stereo Bluetooth haptics format. Speaker transport uses fixed-size 10 ms Opus packets. This is waveform transport, not audio-derived motor rumble.

Your active user needs write access to the matching Sony `hidraw` node. Many gaming distributions already provide it. If the log says it cannot open hidraw, install the included narrowly scoped rule:

```sh
sudo install -m 644 70-artemis-dualsense.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
```

Then disconnect and reconnect the controller. The application does not change permissions or install this rule automatically. Do not run the client as root. An absent or ambiguous output route leaves PCM disabled while ordinary controller input continues. Reconnect after resolving permissions.

## Testing and limitations

Policy boundary tests, lease encoding, packet validation, Bluetooth CRC vectors, full compilation, dependency packaging and headless startup are checked by the release workflow. These checks cannot establish hardware compatibility, game support, audio quality, driver behavior or Bluetooth timing. Both native output paths require physical testing.

The network reorder window is bounded to four packets. ALSA and Bluetooth output are nonblocking, dropping output under pressure rather than building an unbounded backlog. Host acknowledgements and six-second leases gate playback; silence/disconnection clears local playback. Bluetooth audio transport, speaker routing, headset handling, multiple controllers, and automatic adaptation are experimental. Controller microphone forwarding is not included.

## Credits and source

Based on [R3VALATiON/artemis](https://github.com/R3VALATiON/artemis), wjbeckett/Artemis Qt, Moonlight Qt and Sunshine/Apollo. Protocol and native Bluetooth framing/resampling ported from the credited Taveszfito Artemis Android Extended work used in our Android preview, with foundational research from awalol/DS5Dongle, awalol/dualsense-bt-haptics, TechAntohere/CloudPad-Android-Dualsense-Wireless and egormanga/SAxense.

The Extended protocol dependency is pinned to `Taveszfito/moonlight-android` commit `5fd8827d870b860beb000f68cbc76f19a0056546`. Original source and license notices remain intact. The release tag identifies the complete matching source; download it with recursive submodules to rebuild. Qt 6.8.3 uses LGPL/GPL terms supplied by Qt; sources: https://download.qt.io/archive/qt/6.8/6.8.3/single/ . Third-party libraries retain their original licenses.
