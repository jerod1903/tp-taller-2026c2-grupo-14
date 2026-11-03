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

## Compilación y ejecución

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
