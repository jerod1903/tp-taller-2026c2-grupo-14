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
# 6. Agrega el servidor, el cliente y el editor al menú de aplicaciones (Actividades), con
#    archivos .desktop en ~/.local/share/applications.
# 7. Deja los mismos accesos en el Escritorio, para abrir cada programa con doble click.
#
# Muestra una línea por paso; la salida de los comandos va a ~/.cache/stardew-valley-g14/install.log
# (con --verbose se muestra también en pantalla).
#
# Uso:
#   doble click en installer.sh (la primera vez: click derecho, "Permitir ejecutar")
#   bash installer.sh [--skip-deps] [--verbose]
#   curl -fsSL https://raw.githubusercontent.com/jerod1903/tp-taller-2026c2-grupo-14/develop/installer.sh | bash
#     --skip-deps  no instala dependencias (no usa sudo): para cuando ya están.
#     --verbose    muestra la salida de cada paso.

set -euo pipefail

NAME="stardew-valley-g14"
REPO_URL="https://github.com/jerod1903/tp-taller-2026c2-grupo-14.git"
BRANCH="develop"
JOBS="$(nproc)"
BIN="$HOME/.local/bin"
DATA="$HOME/.local/share/$NAME"
APPS="$HOME/.local/share/applications"
# Server y cliente en la misma máquina
PORT=23456
HOST=localhost
LOG="${XDG_CACHE_HOME:-$HOME/.cache}/$NAME/install.log"
VERBOSE=false
# Para el contador [n/total] de cada paso
STEP=0
TOTAL_STEPS=0

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

# Corre un paso con su salida al log (y a la pantalla con --verbose). Si falla, muestra el final de
# la salida y corta, salvo con --soft, que solo avisa. No se puede llamar dentro de un if o un ||:
# ahí bash ignora el set -e de adentro y un comando que falla no cortaría el paso.
run_step() {
    local soft=false
    if [ "$1" = --soft ]; then
        soft=true
        shift
    fi
    STEP=$((STEP + 1))
    local title="[$STEP/$TOTAL_STEPS] $1" step="$2" start=$SECONDS status from
    printf '\n==> %s\n' "$title" >> "$LOG"
    from=$(($(wc -l < "$LOG") + 1))

    set +e
    if [ "$VERBOSE" = true ]; then
        printf '\n==> %s\n' "$title"
        (set -e; "$step") < /dev/null 2>&1 | tee -a "$LOG"
        status=${PIPESTATUS[0]}
    else
        printf '%s... ' "$title"
        (set -e; "$step") < /dev/null >> "$LOG" 2>&1
        status=$?
    fi
    set -e

    local took=$((SECONDS - start))
    if [ "$status" -eq 0 ]; then
        [ "$VERBOSE" = true ] || printf 'listo (%dm %02ds)\n' $((took / 60)) $((took % 60))
        return
    fi
    [ "$VERBOSE" = true ] || echo "falló"
    if [ "$soft" = true ]; then
        TESTS_FAILED=true
        return
    fi
    if [ "$VERBOSE" = false ]; then
        echo
        tail -n +"$from" "$LOG" | tail -n 30
    fi
    echo
    echo "Salida completa en $LOG"
    exit 1
}

install_dependencies() {
    # Si apt está ocupado (p. ej. unattended-upgrades al prender la máquina), espera hasta 5
    # minutos a que se libere en vez de fallar. sudo -n: la contraseña ya se pidió al principio, y
    # con la salida al log no se vería el pedido.
    local lock_wait=(-o DPkg::Lock::Timeout=300)
    sudo -n apt-get "${lock_wait[@]}" update
    sudo -n apt-get "${lock_wait[@]}" install -y "${DEPENDENCIES[@]}"
}

download_source() {
    git clone --depth 1 --branch "$BRANCH" "$REPO_URL" "$SOURCE"
}

build_tests() {
    cmake -S "$SOURCE" -B "$WORK/build-tests" -DCMAKE_BUILD_TYPE=Release \
        -DTALLER_CLIENT=OFF -DTALLER_EDITOR=OFF -DTALLER_TESTS=ON \
        -DTALLER_MAKE_WARNINGS_AS_ERRORS=OFF
    cmake --build "$WORK/build-tests" -j"$JOBS" --target taller_tests
}

run_tests() {
    "$WORK/build-tests/taller_tests" --gtest_brief=1
}

# SDL sin su backend de PipeWire, que no compila con algunas versiones: el sonido sale por
# PulseAudio o ALSA (en Ubuntu, PipeWire también atiende a los programas de PulseAudio).
# Los warnings no cortan la instalación: los controlan la CI y el desarrollo, y un compilador más
# nuevo puede dar avisos nuevos (falsos positivos al optimizar).
build_programs() {
    cmake -S "$SOURCE" -B "$WORK/build-install" -DCMAKE_BUILD_TYPE=Release \
        -DTALLER_INSTALL=ON -DTALLER_TESTS=OFF -DSDL_PIPEWIRE=OFF \
        -DTALLER_MAKE_WARNINGS_AS_ERRORS=OFF
    cmake --build "$WORK/build-install" -j"$JOBS" \
        --target taller_server taller_client taller_editor
}

install_programs() {
    cmake --install "$WORK/build-install"
}

usage() {
    cat << EOF
Instala $NAME: baja el código, instala las dependencias, compila, corre los tests, instala y
agrega el servidor, el cliente y el editor al menú de aplicaciones y al Escritorio.

Uso: bash installer.sh [--skip-deps] [--verbose]
     curl -fsSL <url de installer.sh> | bash -s -- [--skip-deps] [--verbose]

  --skip-deps  no instala dependencias (no usa sudo): para cuando ya están.
  --verbose    muestra la salida de cada paso (si no, queda solo en el log).
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

# El acceso del servidor: lo abre en una terminal, que se cierra con q + Enter, y si termina con
# error espera un Enter para que se pueda leer. Va con la ruta entera: abierto desde el menú no
# tiene ~/.local/bin en el PATH.
# shellcheck disable=SC2016  # las variables entre comillas simples son del acceso, no de acá
write_server_launcher() {
    local path="$1"
    mkdir -p "$(dirname "$path")"
    {
        echo "#!/usr/bin/env bash"
        echo "# Servidor de $NAME en el puerto $PORT; se cierra con q + Enter. Lo generó installer.sh."
        declare -f open_in_terminal
        echo 'open_in_terminal "$@"'
        echo "\"$BIN/$NAME-server\" $PORT || {"
        echo '    status=$?'
        echo '    if [ -n "${STARDEW_IN_TERMINAL:-}" ]; then'
        echo '        read -r -p "Terminó con error. Presioná Enter para cerrar..." || true'
        echo '    fi'
        echo '    exit "$status"'
        echo "}"
    } > "$path"
    chmod +x "$path"
}

# Una entrada del menú de aplicaciones. Exec no expande ~ ni variables: va la ruta entera, entre
# comillas por si tiene espacios. Terminal=false en todas: el servidor abre su propia terminal
# (open_in_terminal en su acceso); con Terminal=true se cerraría al toque si falla.
write_desktop_entry() {
    local id="$1" name="$2" comment="$3" exec="$4"
    # Ícono del juego si viene en Assets; si no, uno genérico del tema
    local icon="applications-games"
    if [ -f "$DATA/Assets/icon.png" ]; then
        icon="$DATA/Assets/icon.png"
    fi
    cat > "$APPS/$NAME-$id.desktop" << EOF
[Desktop Entry]
Type=Application
Name=$name
Comment=$comment
Exec=$exec
Icon=$icon
Terminal=false
Categories=Game;
EOF
}

write_menu_entries() {
    # Junto a los datos, así no depende de nada que esté en el Escritorio
    local server_launcher="$DATA/launchers/server.sh"
    write_server_launcher "$server_launcher"
    mkdir -p "$APPS"
    write_desktop_entry server "Stardew Valley G14 - Servidor" \
        "Servidor en el puerto $PORT; se cierra con q + Enter" "\"$server_launcher\""
    write_desktop_entry client "Stardew Valley G14" "Cliente, conectado a $HOST:$PORT" \
        "\"$BIN/$NAME-client\" $HOST $PORT"
    write_desktop_entry editor "Stardew Valley G14 - Editor de mapas" "Editor de mapas" \
        "\"$BIN/$NAME-editor\""
    # GNOME los ve solo; esto actualiza la caché para otros escritorios
    update-desktop-database "$APPS" 2> /dev/null || true
    echo "Menú de aplicaciones: Stardew Valley G14 (servidor, cliente y editor)"
}

# Accesos directos en el Escritorio a las mismas apps del menú. Se marcan como confiables para que
# GNOME los deje abrir sin el "Permitir ejecutar"; si no puede, se hace a mano la primera vez.
write_desktop_shortcuts() {
    local desktop
    desktop="$(xdg-user-dir DESKTOP 2> /dev/null || true)"
    if [ -z "$desktop" ] || [ "$desktop" = "$HOME" ]; then
        desktop="$HOME/Desktop"
    fi
    mkdir -p "$desktop"
    # Los server.sh, client.sh y editor.sh de instalaciones anteriores quedarían con otro puerto
    for old in server.sh client.sh editor.sh; do
        if grep -qs "Lo generó installer.sh" "$desktop/$old"; then
            rm -f "$desktop/$old"
        fi
    done
    for id in server client editor; do
        local shortcut="$desktop/$NAME-$id.desktop"
        cp "$APPS/$NAME-$id.desktop" "$shortcut"
        chmod +x "$shortcut"
        gio set "$shortcut" metadata::trusted true 2> /dev/null || true
    done
    echo "Accesos directos: servidor, cliente y editor, en $desktop"
}

# Todo dentro de una función: con curl | bash, bash lee el script entero antes de correr nada, y
# los comandos (con su entrada en /dev/null) no se comen el resto del script.
main() {
    local skip_deps=false
    for arg in "$@"; do
        case "$arg" in
            --skip-deps) skip_deps=true ;;
            -v | --verbose) VERBOSE=true ;;
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
        echo "Las dependencias se instalan con sudo: pide tu contraseña una vez."
        sudo -v
    elif ! command -v git > /dev/null; then
        echo "Falta git para bajar el código: instalalo o corré sin --skip-deps" >&2
        exit 1
    fi

    # El código y los builds van a una carpeta temporal, que se borra al salir (aunque falle algo)
    WORK="$(mktemp -d -t "$NAME-XXXXXX")"
    SOURCE="$WORK/source"

    # Las configs que ya están no se pisan (pueden tener cambios del usuario): se avisa al final,
    # porque una config de otra versión puede no tener los valores nuevos
    local config_dir="$HOME/.config/$NAME"
    local kept=()
    for config in server_config.toml client_config.toml; do
        if [ -f "$config_dir/$config" ]; then
            kept+=("$config")
        fi
    done

    mkdir -p "$(dirname "$LOG")"
    : > "$LOG"
    echo
    TOTAL_STEPS=7
    if [ "$skip_deps" = false ]; then
        TOTAL_STEPS=8
        run_step "Instalando las dependencias" install_dependencies
    fi
    run_step "Bajando el código ($BRANCH)" download_source
    run_step "Compilando los tests" build_tests
    # PROVISORIO: un test que falla no corta la instalación, se avisa al final. Volver a cortar
    # (sacar el --soft) cuando pasen todos.
    TESTS_FAILED=false
    run_step --soft "Corriendo los tests" run_tests
    run_step "Compilando el servidor, el cliente y el editor" build_programs
    run_step "Instalando" install_programs
    run_step "Agregando al menú de aplicaciones" write_menu_entries
    run_step "Creando los accesos directos del Escritorio" write_desktop_shortcuts

    echo
    echo "Listo. Para jugar: abrí \"Stardew Valley G14 - Servidor\" y después \"Stardew Valley G14\""
    echo "(uno por jugador), desde Actividades o desde el Escritorio."
    case ":$PATH:" in
        *":$BIN:"*) ;;
        *) echo "Ojo: ~/.local/bin no está en tu PATH; agregalo para correrlos por nombre." ;;
    esac
    if [ "$TESTS_FAILED" = true ]; then
        echo "Ojo: fallaron tests (ver \"Corriendo los tests\" en $LOG)."
    fi
    if [ "${#kept[@]}" -gt 0 ]; then
        echo "Ojo: quedaron tus configs de antes en $config_dir (${kept[*]}). Si les faltan valores"
        echo "nuevos, borralas y volvé a correr el instalador."
    fi
}

main "$@"
