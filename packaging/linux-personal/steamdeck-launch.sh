#!/usr/bin/env bash
# Unofficial personal testing launcher. No root or SteamOS changes.
set -euo pipefail
here="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# Game Mode uses gamescope/Xwayland. Desktop Mode may select native Wayland.
if [[ "${SteamDeck:-0}" == 1 || "${XDG_CURRENT_DESKTOP:-}" == *gamescope* || "${XDG_SESSION_DESKTOP:-}" == *gamescope* ]]; then
  if [[ -n "${DISPLAY:-}" ]]; then
    export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"
    export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}"
  fi
fi
# Keep the host graphics driver and its data directories together.
if [[ -z "${LIBVA_DRIVERS_PATH:-}" && -d /usr/lib/dri ]]; then export LIBVA_DRIVERS_PATH=/usr/lib/dri; fi
if [[ -x "$here/AppRun" ]]; then
  exec "$here/AppRun" "$@"
fi
image="$here/Artemis-SteamDeck-Personal-Testing-0.3.1-preview.1-x86_64.AppImage"
if [[ -f "$image" ]]; then
  export APPIMAGE_EXTRACT_AND_RUN=1
  exec "$image" "$@"
fi
printf '%s\n' 'Keep this launcher inside the extracted portable folder or beside the matching AppImage.' >&2
exit 1
