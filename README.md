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
