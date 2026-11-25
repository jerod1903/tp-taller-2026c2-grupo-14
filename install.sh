#!/usr/bin/env bash
# Instalador de Stardew Valley - Taller de Programación, grupo 14.
#
# Se corre suelto, sin tener el código: lo baja de GitHub a una carpeta temporal y la borra al
# terminar, así solo queda lo instalado.
#
# 1. Instala las dependencias del sistema con apt (pide la contraseña de sudo una sola vez).
# 2. Baja el código de la rama main.
# 3. Compila y corre los tests.
# 4. Compila el servidor, el cliente y el editor.
# 5. Copia los binarios a ~/.local/bin, Assets/ y maps/ a ~/.local/share/stardew-valley-g14/ y las
#    configs a ~/.config/stardew-valley-g14/ (las que ya están no se pisan).
#
# Uso:
#   curl -fsSL https://raw.githubusercontent.com/jerod1903/tp-taller-2026c2-grupo-14/main/install.sh | bash
#   bash install.sh [--skip-deps]
#     --skip-deps  no instala dependencias (no usa sudo): para cuando ya están.

set -euo pipefail

NAME="stardew-valley-g14"
REPO_URL="https://github.com/jerod1903/tp-taller-2026c2-grupo-14.git"
BRANCH="main"
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

usage() {
    cat << EOF
Instala $NAME: baja el código, instala las dependencias, compila, corre los tests e instala.

Uso: bash install.sh [--skip-deps]
     curl -fsSL <url de install.sh> | bash -s -- [--skip-deps]

  --skip-deps  no instala dependencias (no usa sudo): para cuando ya están.
EOF
}

# Todo dentro de una función: con curl | bash, bash lee el script entero antes de correr nada, y
# los comandos (con su entrada en /dev/null) no se comen el resto del script.
main() {
    local skip_deps=false
    for arg in "$@"; do
        case "$arg" in
            --skip-deps) skip_deps=true ;;
            -h | --help)
                usage
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
                "con --skip-deps" >&2
            exit 1
        fi
        step "Instalando dependencias (sudo pide tu contraseña una vez)"
        sudo -v
        sudo apt-get update < /dev/null
        sudo apt-get install -y "${DEPENDENCIES[@]}" < /dev/null
    elif ! command -v git > /dev/null; then
        echo "Falta git para bajar el código: instalalo o corré sin --skip-deps" >&2
        exit 1
    fi

    # El código y los builds van a una carpeta temporal, que se borra al salir (aunque falle algo)
    WORK="$(mktemp -d -t "$NAME-XXXXXX")"
    trap 'rm -rf "$WORK"' EXIT
    local source="$WORK/source"

    step "Bajando el código ($BRANCH)"
    git clone --depth 1 --branch "$BRANCH" "$REPO_URL" "$source" < /dev/null

    step "Compilando los tests"
    cmake -S "$source" -B "$WORK/build-tests" -DCMAKE_BUILD_TYPE=Release \
        -DTALLER_CLIENT=OFF -DTALLER_EDITOR=OFF -DTALLER_TESTS=ON \
        -DTALLER_MAKE_WARNINGS_AS_ERRORS=OFF < /dev/null
    cmake --build "$WORK/build-tests" -j"$JOBS" --target taller_tests < /dev/null

    step "Corriendo los tests"
    "$WORK/build-tests/taller_tests" --gtest_brief=1 < /dev/null

    step "Compilando el servidor, el cliente y el editor"
    # SDL sin su backend de PipeWire, que no compila con algunas versiones: el sonido sale por
    # PulseAudio o ALSA (en Ubuntu, PipeWire también atiende a los programas de PulseAudio).
    # Los warnings no cortan la instalación: los controlan la CI y el desarrollo, y un compilador
    # más nuevo puede dar avisos nuevos (falsos positivos al optimizar).
    cmake -S "$source" -B "$WORK/build-install" -DCMAKE_BUILD_TYPE=Release \
        -DTALLER_INSTALL=ON -DTALLER_TESTS=OFF -DSDL_PIPEWIRE=OFF \
        -DTALLER_MAKE_WARNINGS_AS_ERRORS=OFF < /dev/null
    cmake --build "$WORK/build-install" -j"$JOBS" \
        --target taller_server taller_client taller_editor < /dev/null

    # Las configs que ya están no se pisan (pueden tener cambios del usuario): se avisa al final,
    # porque una config de otra versión puede no tener los valores nuevos
    local config_dir="$HOME/.config/$NAME"
    local kept=()
    for config in server_config.toml client_config.toml; do
        if [ -f "$config_dir/$config" ]; then
            kept+=("$config")
        fi
    done

    step "Instalando"
    cmake --install "$WORK/build-install" < /dev/null

    step "Listo"
    echo "Programas: $NAME-server, $NAME-client y $NAME-editor, en ~/.local/bin"
    echo "Datos: ~/.local/share/$NAME    Configs: ~/.config/$NAME"
    case ":$PATH:" in
        *":$HOME/.local/bin:"*) ;;
        *) echo "Ojo: ~/.local/bin no está en tu PATH; agregalo para correrlos por nombre." ;;
    esac
    if [ "${#kept[@]}" -gt 0 ]; then
        echo "Ojo: quedaron las configs que ya tenías en $config_dir: ${kept[*]}."
        echo "Si son de una versión anterior pueden faltarles valores nuevos; para instalar las de"
        echo "esta versión, borralas y volvé a correr el instalador."
    fi
}

main "$@"
