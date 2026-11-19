#!/usr/bin/env bash
# Instalador de Stardew Valley - Taller de Programación, grupo 14.
#
# 1. Instala las dependencias del sistema con apt (pide la contraseña de sudo una sola vez).
# 2. Compila y corre los tests contra los archivos del repo.
# 3. Compila el servidor, el cliente y el editor para instalar.
# 4. Copia los binarios a ~/.local/bin, Assets/ y maps/ a ~/.local/share/stardew-valley-g14/ y las
#    configs a ~/.config/stardew-valley-g14/ (las que ya están no se pisan).
#
# Uso: ./install.sh [--skip-deps]
#   --skip-deps  no instala dependencias (no usa sudo): para cuando ya están.

set -euo pipefail

NAME="stardew-valley-g14"
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TESTS_BUILD="$REPO/build-tests"
INSTALL_BUILD="$REPO/build-install"
JOBS="$(nproc)"

# Para compilar el proyecto (cmake, Qt) y SDL2 con sus módulos, que se bajan y compilan desde el
# código fuente: video (X11, Wayland), audio (ALSA, PulseAudio), formatos de sonido del mixer y
# fuentes del ttf.
DEPENDENCIES=(
    build-essential cmake git pkg-config
    qt6-base-dev libgl-dev
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxss-dev libxinerama-dev
    libxkbcommon-dev libwayland-dev wayland-protocols libegl-dev libgles-dev
    libasound2-dev libpulse-dev
    libopus-dev libopusfile-dev libxmp-dev libfluidsynth-dev fluidsynth libwavpack-dev wavpack
    libfreetype-dev
)

step() {
    printf '\n==> %s\n' "$1"
}

skip_deps=false
for arg in "$@"; do
    case "$arg" in
        --skip-deps) skip_deps=true ;;
        -h | --help)
            sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "Opción desconocida: $arg (ver --help)" >&2
            exit 1
            ;;
    esac
done

if [ "$skip_deps" = false ]; then
    if ! command -v apt-get > /dev/null; then
        echo "No hay apt-get: las dependencias son para Ubuntu 24.04. Si ya las tenés, corré" \
            "./install.sh --skip-deps" >&2
        exit 1
    fi
    step "Instalando dependencias (sudo pide tu contraseña una vez)"
    sudo -v
    sudo apt-get update
    sudo apt-get install -y "${DEPENDENCIES[@]}"
fi

step "Compilando los tests"
cmake -S "$REPO" -B "$TESTS_BUILD" -DCMAKE_BUILD_TYPE=Release \
    -DTALLER_CLIENT=OFF -DTALLER_EDITOR=OFF -DTALLER_TESTS=ON \
    -DTALLER_MAKE_WARNINGS_AS_ERRORS=OFF
cmake --build "$TESTS_BUILD" -j"$JOBS" --target taller_tests

step "Corriendo los tests"
"$TESTS_BUILD/taller_tests" --gtest_brief=1

step "Compilando el servidor, el cliente y el editor"
# SDL sin su backend de PipeWire, que no compila con algunas versiones: el sonido sale por
# PulseAudio o ALSA (en Ubuntu, PipeWire también atiende a los programas de PulseAudio).
# Los warnings no cortan la instalación: los controlan la CI y el desarrollo, y un compilador más
# nuevo puede dar avisos nuevos (falsos positivos al optimizar).
cmake -S "$REPO" -B "$INSTALL_BUILD" -DCMAKE_BUILD_TYPE=Release \
    -DTALLER_INSTALL=ON -DTALLER_TESTS=OFF -DSDL_PIPEWIRE=OFF \
    -DTALLER_MAKE_WARNINGS_AS_ERRORS=OFF
cmake --build "$INSTALL_BUILD" -j"$JOBS" --target taller_server taller_client taller_editor

step "Instalando"
cmake --install "$INSTALL_BUILD"

step "Listo"
echo "Programas: $NAME-server, $NAME-client y $NAME-editor, en ~/.local/bin"
echo "Datos: ~/.local/share/$NAME    Configs: ~/.config/$NAME"
case ":$PATH:" in
    *":$HOME/.local/bin:"*) ;;
    *) echo "Ojo: ~/.local/bin no está en tu PATH; agregalo para correrlos por nombre." ;;
esac
