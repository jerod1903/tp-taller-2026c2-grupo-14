# tp-taller-2026c2-grupo-14

Trabajo Práctico Final grupal de Taller de Programación I - Cátedra Veiga.

2026 C2

## Estructura

```text
.
├── CMakeLists.txt              # Definición de targets, dependencias (SDL2, SDL2pp, GoogleTest) y opciones
├── Makefile                    # Atajos para compilar y correr los tests
├── cmake/
│   └── CompilerWarnings.cmake  # Warnings del compilador aplicados a cada target
├── common/                     # Biblioteca estática compartida (taller_common)
├── client/                     # Cliente (taller_client), usa SDL2pp
├── server/                     # Servidor (taller_server)
├── editor/                     # Editor (taller_editor), usa SDL2pp
└── tests/                      # Tests con GoogleTest (taller_tests)
```

Cada carpeta de código tiene su propio `CMakeLists.txt` donde se listan sus archivos fuente con `target_sources`. `client`, `server`, `editor` y `tests` se linkean contra `taller_common`.

Cada target se puede activar o desactivar con las opciones de CMake `TALLER_CLIENT`, `TALLER_SERVER`, `TALLER_EDITOR` y `TALLER_TESTS` (todas en `ON` por defecto). `TALLER_MAKE_WARNINGS_AS_ERRORS` controla si los warnings se tratan como errores.

## Instalación

En Ubuntu 24.04 el instalador baja el juego de la rama `main` a una carpeta temporal, instala las dependencias (pide sudo), compila, corre los tests, instala y borra la carpeta.

```bash
curl -fsSL https://raw.githubusercontent.com/jerod1903/tp-taller-2026c2-grupo-14/main/install.sh | bash
```

Con `--skip-deps` hace lo mismo sin instalar dependencias ni usar sudo (para cuando ya están):

```bash
curl -fsSL https://raw.githubusercontent.com/jerod1903/tp-taller-2026c2-grupo-14/main/install.sh | bash -s -- --skip-deps
```

Para desinstalar (borra lo instalado, salvo las partidas guardadas):

```bash
curl -fsSL https://raw.githubusercontent.com/jerod1903/tp-taller-2026c2-grupo-14/main/uninstall.sh | bash
```

Si ya tenés el repo o bajaste los scripts, también se corren como `bash install.sh [--skip-deps]` y `bash uninstall.sh`.

Queda instalado:

- `~/.local/bin/`: `stardew-valley-g14-server`, `stardew-valley-g14-client` y `stardew-valley-g14-editor`.
- `~/.local/share/stardew-valley-g14/`: `Assets/` y `maps/` (donde el editor guarda los mapas nuevos).
- `~/.config/stardew-valley-g14/`: `server_config.toml` y `client_config.toml`. Si ya existen, reinstalar no los pisa.

Para jugar:

```bash
stardew-valley-g14-server 8080              # en una terminal; se cierra con q + Enter
stardew-valley-g14-client localhost 8080    # en otra, una por jugador
stardew-valley-g14-editor
```

## Cheats

Se escriben en el chat de la partida (Enter para abrirlo). Le llegan solo al que los usa; si están mal escritos, el servidor le contesta cómo se usan.

| Cheat | Qué hace |
|---|---|
| `$gold [cantidad]` | Suma esa cantidad de oro. Sin cantidad, la de `gold` en `[cheats]` de `server_config.toml`. |
| `$seeds [cantidad]` | Da esa cantidad de semillas de cada cultivo, las que entren en el inventario. Sin cantidad, la de `seeds` en `[cheats]`. |
| `$sleep` | Termina el día en el acto, como si fueran las 2:00: aparece la votación para pasar al día siguiente. |
| `$finish` | Provisorio: completa el juego como si se terminara el centro comunitario. Todos los conectados ven la pantalla de felicitación; la partida sigue. |

## Compilación y ejecución

Para desarrollar, los programas compilados en `build/` leen `Assets/`, `maps/` y las configs del repo.

```bash
make compile-debug   # compila en build/ (modo Debug)
make run-tests       # compila y corre ./build/taller_tests
make clean           # borra build/
```

Los binarios quedan en `build/`: `taller_client`, `taller_server`, `taller_editor` y `taller_tests`.

## Referencias

Di Paola, M. (2024). _tp-grupal-template_ [Código Fuente](https://github.com/Taller-de-Programacion-TPs/tp-grupal-template). GitHub.

- **Archivos utilizados:**
  - `CMakeLists.txt`
  - `Makefile`
  - `cmake/CompilerWarnings.cmake`
  - `client/CMakeLists.txt`
  - `common/CMakeLists.txt`
  - `editor/CMakeLists.txt`
  - `server/CMakeLists.txt`
  - `tests/CMakeLists.txt`
- **Licencia:** no especificada

Di Paola, M. (2025). _hands-on-sockets-in-cpp_ [Código Fuente](https://github.com/eldipa/hands-on-sockets-in-cpp). GitHub.

- **Archivos utilizados:**
  - `socket.cpp`
  - `socket.h`
  - `resolver.h`
  - `resolver.cpp`
  - `resolvererror.h`
  - `resolvererror.cpp`
  - `liberror.h`
  - `liberror.cpp`
- **Licencia:** GNU General Public License v2.0 (GPL v2)

Di Paola, M. (2024). _hands-on-threads_ [Código Fuente](https://github.com/eldipa/hands-on-threads). GitHub.

- **Archivos utilizados:**
  - `thread.h`
  - `queue.h`
- **Licencia:** GNU General Public License v2.0 (GPL v2)

## Assets de terceros

- **Fondos, botones e inputs del menú:** Pixel Art Wooden GUI v1 por Narik (Soulofkiran), licencia CC0. Ver [`Assets/Menu/LICENSE.txt`](Assets/Menu/LICENSE.txt). Fondos: Cozy Village Backgrounds (licencia pendiente de verificar).
- **Hojas de personaje (`StardewValley_Character_*`):** provistas por la cátedra. No se incluyen en el repositorio.
