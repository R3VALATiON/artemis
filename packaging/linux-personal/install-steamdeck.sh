#!/usr/bin/env bash
# Register the already-extracted portable client in the current user's menu.
set -euo pipefail
here="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
[[ -x "$here/AppRun" ]] || { echo 'Extract the portable archive and run this script inside its folder.' >&2; exit 1; }
[[ "$here" != *$'\n'* ]] || exit 1
mkdir -p "$HOME/.local/share/applications"
python3 - "$here" "$HOME/.local/share/applications/artemis-ds-personal.desktop" <<'PY'
import pathlib,sys
root=pathlib.Path(sys.argv[1])
launch=str(root/'steamdeck-launch.sh').replace('\\','\\\\').replace('"','\\"').replace('`','\\`').replace('$','\\$').replace('%','%%')
pathlib.Path(sys.argv[2]).write_text('[Desktop Entry]\nType=Application\nName=Artemis DS Steam Deck Personal Test\nComment=Unofficial personal development and testing preview\nExec="'+launch+'"\nIcon='+str(root/'usr/share/icons/hicolor/256x256/apps/artemis-personal.png')+'\nTerminal=false\nCategories=Game;Network;\n')
PY
printf '%s\n' 'Menu entry created. Keep this extracted folder in place.' 'In Desktop Mode, add Artemis DS Steam Deck Personal Test to Steam as a non-Steam game.' 'Leave Proton/forced compatibility tools off for this native Linux build.'
