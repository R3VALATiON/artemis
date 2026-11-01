#!/usr/bin/env bash
# Optional read-only diagnostic. Does not install packages or alter SteamOS.
set -euo pipefail
here="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
[[ -f "$here/usr/bin/artemis" ]] || { echo 'Run this inside the extracted portable folder.' >&2; exit 1; }
[[ "$(uname -m)" == x86_64 ]] || { echo 'This package requires x86-64.' >&2; exit 1; }
printf '%s\n' 'Artemis DS personal preview — read-only Steam Deck preflight'
if [[ -f /etc/os-release ]]; then grep -E '^(NAME|VERSION_ID|BUILD_ID)=' /etc/os-release; fi
printf 'Kernel: '; uname -r
printf 'Session: %s; desktop: %s\n' "${XDG_SESSION_TYPE:-unknown}" "${XDG_CURRENT_DESKTOP:-unknown}"
export LD_LIBRARY_PATH="$here/usr/lib:${LD_LIBRARY_PATH:-}"
fail=0
while IFS= read -r -d '' item; do
  if [[ "$(head -c 4 "$item" 2>/dev/null)" != $'\x7fELF' ]]; then continue; fi
  result="$(ldd "$item" 2>&1 || true)"
  if grep -q 'not found' <<< "$result"; then printf '%s\n%s\n' "$item" "$result"; fail=1; fi
done < <(find "$here/usr" -type f -print0)
printf '\nMain client dependency resolution:\n'
ldd "$here/usr/bin/artemis"
printf '\nDualSense hidraw access (absence means no physical controller is connected):\n'
for path in /sys/class/hidraw/hidraw*/device/uevent; do
  [[ -f "$path" ]] || continue
  if grep -Eq '^HID_ID=000[35]:0000054[Cc]:00000([Cc][Ee]6|[Dd][Ff]2)$' "$path"; then
    node="/dev/$(basename "$(dirname "$(dirname "$path")")")"
    if [[ -w "$node" ]]; then printf '%s writable\n' "$node"; else printf '%s permission required\n' "$node"; fi
  fi
done
printf '\nPreflight complete. Physical streaming, GPU decode, suspend/resume and haptics still need an on-device test.\n'
exit "$fail"
