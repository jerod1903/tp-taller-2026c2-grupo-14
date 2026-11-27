#!/usr/bin/env bash
# Instalador de Stardew Valley - Taller de Programación, grupo 14.
#
# Se corre suelto, sin tener el código: lo baja de GitHub a una carpeta temporal y la borra al
# terminar, así solo queda lo instalado. Se puede abrir con doble click: si no tiene una terminal,
# se abre en una (para pedir la contraseña de sudo y mostrar cómo va).
#
# 1. Instala las dependencias del sistema con apt (pide la contraseña de sudo una sola vez).
# 2. Baja el código de la rama develop.
# 3. Compila y corre los tests.
# 4. Compila el servidor, el cliente y el editor.
# 5. Copia los binarios a ~/.local/bin, Assets/ y maps/ a ~/.local/share/stardew-valley-g14/ y las
#    configs a ~/.config/stardew-valley-g14/ (las que ya están no se pisan).
# 6. Deja en el Escritorio server.sh, client.sh y editor.sh, para abrir cada programa con doble
#    click.
#
# Uso:
#   doble click en installer.sh (la primera vez: click derecho, "Permitir ejecutar")
#   bash installer.sh [--skip-deps]
#   curl -fsSL https://raw.githubusercontent.com/jerod1903/tp-taller-2026c2-grupo-14/develop/installer.sh | bash
#     --skip-deps  no instala dependencias (no usa sudo): para cuando ya están.

set -euo pipefail

NAME="stardew-valley-g14"
REPO_URL="https://github.com/jerod1903/tp-taller-2026c2-grupo-14.git"
BRANCH="develop"
JOBS="$(nproc)"
BIN="$HOME/.local/bin"
# Server y cliente en la misma máquina
PORT=8080
HOST=localhost

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
Instala $NAME: baja el código, instala las dependencias, compila, corre los tests, instala y deja
en el Escritorio server.sh, client.sh y editor.sh.

Uso: bash installer.sh [--skip-deps]
     curl -fsSL <url de installer.sh> | bash -s -- [--skip-deps]

  --skip-deps  no instala dependencias (no usa sudo): para cuando ya están.
EOF
}

# Abierto con doble click no hay terminal: se vuelve a correr adentro de una, que queda abierta al
# terminar para leer cómo salió. La de Ubuntu primero; si no hay ninguna, sigue sin terminal.
open_in_terminal() {
    if [ -t 1 ] || [ -n "${STARDEW_IN_TERMINAL:-}" ] || [ ! -f "$0" ]; then
        return
    fi
    local script
    script="$(realpath "$0")"
    local run=(env STARDEW_IN_TERMINAL=1 bash "$script" "$@")
    if command -v gnome-terminal > /dev/null; then
        exec gnome-terminal -- "${run[@]}"
    elif command -v konsole > /dev/null; then
        exec konsole -e "${run[@]}"
    elif command -v xfce4-terminal > /dev/null; then
        exec xfce4-terminal -x "${run[@]}"
    elif command -v x-terminal-emulator > /dev/null; then
        exec x-terminal-emulator -e "${run[@]}"
    elif command -v xterm > /dev/null; then
        exec xterm -e "${run[@]}"
    fi
}

# Al salir, bien o mal: se borra la carpeta temporal y, si se abrió su propia terminal, espera un
# Enter para que no se cierre antes de leer el resultado
on_exit() {
    local status=$?
    if [ -n "${WORK:-}" ]; then
        rm -rf "$WORK"
    fi
    if [ -n "${STARDEW_IN_TERMINAL:-}" ]; then
        if [ "$status" -ne 0 ]; then
            echo
            echo "La instalación falló (ver arriba)."
        fi
        read -r -p "Presioná Enter para cerrar..." || true
    fi
}

# Un acceso del Escritorio: corre el programa instalado (con la ruta entera: abierto con doble click
# no tiene ~/.local/bin en el PATH). `terminal`: si se abre en una terminal, como el servidor, que
# se cierra con q + Enter; si el programa termina con error, la terminal espera un Enter para que
# se pueda leer.
# shellcheck disable=SC2016  # las variables entre comillas simples son del acceso, no de acá
write_launcher() {
    local path="$1" description="$2" command="$3" terminal="$4"
    {
        echo "#!/usr/bin/env bash"
        echo "# $description. Lo generó installer.sh."
        if [ "$terminal" = true ]; then
            declare -f open_in_terminal
            echo 'open_in_terminal "$@"'
            echo "$command || {"
            echo '    status=$?'
            echo '    if [ -n "${STARDEW_IN_TERMINAL:-}" ]; then'
            echo '        read -r -p "Terminó con error. Presioná Enter para cerrar..." || true'
            echo '    fi'
            echo '    exit "$status"'
            echo "}"
        else
            echo "exec $command"
        fi
    } > "$path"
    chmod +x "$path"
}

write_launchers() {
    local desktop
    desktop="$(xdg-user-dir DESKTOP 2> /dev/null || true)"
    if [ -z "$desktop" ] || [ "$desktop" = "$HOME" ]; then
        desktop="$HOME/Desktop"
    fi
    mkdir -p "$desktop"
    write_launcher "$desktop/server.sh" \
        "Servidor de $NAME en el puerto $PORT; se cierra con q + Enter" \
        "\"$BIN/$NAME-server\" $PORT" true
    write_launcher "$desktop/client.sh" "Cliente de $NAME, conectado a $HOST:$PORT" \
        "\"$BIN/$NAME-client\" $HOST $PORT" false
    write_launcher "$desktop/editor.sh" "Editor de mapas de $NAME" "\"$BIN/$NAME-editor\"" false
    echo "Accesos: server.sh, client.sh y editor.sh, en $desktop"
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

    open_in_terminal "$@"
    trap on_exit EXIT

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
    local source="$WORK/source"

    step "Bajando el código ($BRANCH)"
    git clone --depth 1 --branch "$BRANCH" "$REPO_URL" "$source" < /dev/null

    step "Compilando los tests"
    cmake -S "$source" -B "$WORK/build-tests" -DCMAKE_BUILD_TYPE=Release \
        -DTALLER_CLIENT=OFF -DTALLER_EDITOR=OFF -DTALLER_TESTS=ON \
        -DTALLER_MAKE_WARNINGS_AS_ERRORS=OFF < /dev/null
    cmake --build "$WORK/build-tests" -j"$JOBS" --target taller_tests < /dev/null

    step "Corriendo los tests"
    # PROVISORIO: un test que falla no corta la instalación, se avisa al final. Volver a cortar
    # (sacar el `|| tests_failed=true`) cuando pasen todos.
    local tests_failed=false
    "$WORK/build-tests/taller_tests" --gtest_brief=1 < /dev/null || tests_failed=true

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

    step "Creando los accesos del Escritorio"
    write_launchers

    step "Listo"
    echo "Programas: $NAME-server, $NAME-client y $NAME-editor, en ~/.local/bin"
    echo "Datos: ~/.local/share/$NAME    Configs: ~/.config/$NAME"
    echo "Para jugar: doble click en server.sh y después en client.sh (uno por jugador). La primera"
    echo "vez, click derecho en cada uno y \"Permitir ejecutar\"."
    case ":$PATH:" in
        *":$BIN:"*) ;;
        *) echo "Ojo: ~/.local/bin no está en tu PATH; agregalo para correrlos por nombre." ;;
    esac
    if [ "$tests_failed" = true ]; then
        echo "Ojo: fallaron tests (ver arriba, en \"Corriendo los tests\")."
    fi
    if [ "${#kept[@]}" -gt 0 ]; then
        echo "Ojo: quedaron las configs que ya tenías en $config_dir: ${kept[*]}."
        echo "Si son de una versión anterior pueden faltarles valores nuevos; para instalar las de"
        echo "esta versión, borralas y volvé a correr el instalador."
    fi
}

main "$@"
