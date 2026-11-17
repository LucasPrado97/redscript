#!/bin/bash
# Builds <dir>/r6 the way ~/Downloads/CyPort-scripts/build-scripts.sh does: the original blob as
# r6/cache/final.redscripts, r6/scripts from the mods in mods.txt and the plugin folders in plugins.txt.
# Usage: make_workdir.sh <dir>
set -euo pipefail
W="${1:?work dir}"
SCRIPTS="$HOME/Downloads/CyPort-scripts"
MODS_DIR="$HOME/Downloads/CyPort/Mods Nexus"
ORIGINAL="$MODS_DIR/_backup-iphone-2026-11-15/final.redscripts.original"
rm -rf "$W"; mkdir -p "$W/r6/cache" "$W/r6/scripts"
cp "$ORIGINAL" "$W/r6/cache/final.redscripts"
while IFS= read -r m; do
  [[ -z "$m" || "$m" == \#* ]] && continue
  z=$(ls "$MODS_DIR" | grep -F "$m" | head -1)
  [ -n "$z" ] || { echo "missing: $m"; exit 1; }
  bsdtar -xf "$MODS_DIR/$z" -C "$W" "r6/scripts"
done < "$SCRIPTS/mods.txt"
while IFS='=' read -r name dir; do
  [[ -z "$name" || "$name" == \#* ]] && continue
  mkdir -p "$W/r6/scripts/$name"; cp -R "${dir/#\~/$HOME}/." "$W/r6/scripts/$name/"
done < "$SCRIPTS/plugins.txt"
echo "$W: $(find "$W/r6/scripts" -name '*.reds' | wc -l | tr -d ' ') .reds, $(du -sh "$W/r6/scripts" | cut -f1)"
