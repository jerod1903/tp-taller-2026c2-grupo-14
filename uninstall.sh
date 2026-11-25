#!/usr/bin/env bash
# Desinstala lo que puso install.sh: los binarios de ~/.local/bin, los datos de
# ~/.local/share/stardew-valley-g14 (salvo las partidas guardadas, en saves/) y las configs de
# ~/.config/stardew-valley-g14. Las dependencias del sistema quedan instaladas.
#
# Se corre suelto, sin el código:
#   curl -fsSL https://raw.githubusercontent.com/jerod1903/tp-taller-2026c2-grupo-14/main/uninstall.sh | bash

set -euo pipefail

NAME="stardew-valley-g14"
DATA="$HOME/.local/share/$NAME"

rm -f "$HOME/.local/bin/$NAME-server" "$HOME/.local/bin/$NAME-client" \
    "$HOME/.local/bin/$NAME-editor"
rm -rf "$DATA/Assets" "$DATA/maps"
rm -rf "$HOME/.config/$NAME"
# Si no quedaron partidas guardadas, la carpeta vacía tampoco
rmdir "$DATA" 2>/dev/null || true

echo "Desinstalado."
if [ -d "$DATA/saves" ]; then
    echo "Las partidas guardadas siguen en $DATA/saves"
fi
