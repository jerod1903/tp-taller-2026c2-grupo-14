cmake_minimum_required(VERSION 3.24)

# Las dependencias bajadas por URL toman la fecha de extracción (y no la del archivo)
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# C++20. Con extensiones (gnu++20): el código de sockets de la cátedra (common/net) usa funciones
# POSIX, como strerror_r, que con C++20 estricto no se declaran.
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_EXTENSIONS ON)

project(stardew_valley_g14
    VERSION 1.0
    HOMEPAGE_URL "https://github.com/jerod1903/tp-taller-2026c2-grupo-14"
    LANGUAGES CXX
    )

option(TALLER_TESTS "Enable / disable tests." ON)
option(TALLER_CLIENT "Enable / disable client program." ON)
option(TALLER_SERVER "Enable / disable server program." ON)
option(TALLER_EDITOR "Enable / disable editor program." ON)
option(TALLER_MAKE_WARNINGS_AS_ERRORS "Enable / disable warnings as errors." ON)
# Para instalar (lo usa install.sh): los programas buscan sus datos en ~/.local/share/TALLER_NAME y
# sus configs en ~/.config/TALLER_NAME. Sin esto, los buscan en el repo, para desarrollar.
option(TALLER_INSTALL "Build to install in ~/.local and ~/.config." OFF)
set(TALLER_NAME stardew-valley-g14)

if(TALLER_INSTALL)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install prefix." FORCE)
    set(TALLER_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/${TALLER_NAME}")
    set(TALLER_CONFIG_DIR "$ENV{HOME}/.config/${TALLER_NAME}")
else()
    set(TALLER_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(TALLER_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Las dependencias que no vienen del sistema se bajan y compilan con el proyecto. Las del sistema
# (Qt6, y las que necesita SDL para compilarse) las instala install.sh.
include(FetchContent)
include(cmake/CompilerWarnings.cmake)

# Common section
# ---------------
# Biblioteca con lo compartido entre cliente, servidor y editor (protocolo, DTOs, mapas, logger).
# Sus archivos se listan en common/CMakeLists.txt.
add_library(taller_common STATIC)
add_subdirectory(common/)
set_project_warnings(taller_common ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)

# PUBLIC: quien linkea contra taller_common incluye desde la raíz del repo ("common/...")
target_include_directories(taller_common PUBLIC .)

# toml++ (header-only) para leer las configs y los mapas
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tomlplusplus)
target_link_libraries(taller_common PUBLIC tomlplusplus::tomlplusplus)

# Rutas absolutas, para que cliente, servidor y editor no dependan del cwd: maps/ (los mapas, que el
# editor también escribe), saves/ (las partidas guardadas), Assets/ y la carpeta de
# server_config.toml y client_config.toml. Son las del repo o las instaladas, según TALLER_INSTALL.
target_compile_definitions(taller_common PUBLIC
    TALLER_MAPS_DIR="${TALLER_DATA_DIR}/maps"
    TALLER_SAVES_DIR="${TALLER_DATA_DIR}/saves"
    TALLER_ASSETS_DIR="${TALLER_DATA_DIR}/Assets"
    TALLER_CONFIG_DIR="${TALLER_CONFIG_DIR}")

# SDL section
# ---------------
# SDL2 con image, mixer y ttf, y libSDL2pp, para el juego del cliente (el editor es solo Qt)
if(TALLER_CLIENT)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.8
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG release-2.8.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG release-2.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-2.22.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        libSDL2pp
        OVERRIDE_FIND_PACKAGE
        URL https://github.com/libSDL2pp/libSDL2pp/archive/cc198c9a5657048bee67ece82de620b2d5661084.zip
    )

    # Como subproyecto, libSDL2pp solo envuelve SDL_image, SDL_mixer y SDL_ttf si el proyecto padre
    # lo pide.
    set(SDL2PP_WITH_IMAGE ON)
    set(SDL2PP_WITH_MIXER ON)
    set(SDL2PP_WITH_TTF ON)
    # Para instalar, estáticas dentro del cliente: install.sh borra el build al terminar, y con
    # bibliotecas compartidas el cliente las buscaría ahí.
    if(TALLER_INSTALL)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
        set(SDL2PP_STATIC ON CACHE BOOL "" FORCE)
    endif()
    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_mixer SDL2_ttf libSDL2pp)
    # Estáticas, SDL_image, SDL_mixer y SDL_ttf se exportan como SDL2_x::SDL2_x-static, y libSDL2pp
    # linkea contra el nombre de las compartidas
    foreach(lib SDL2_image SDL2_mixer SDL2_ttf)
        if(NOT TARGET ${lib}::${lib})
            add_library(${lib}::${lib} ALIAS ${lib})
        endif()
    endforeach()
endif()

# Program section
# ---------------
# Cada programa lista sus archivos en su carpeta (client/, server/, editor/) y linkea contra
# taller_common.
if(TALLER_CLIENT)
    # Qt6 Widgets para los menús de antes de la partida (del sistema: qt6-base-dev)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    add_executable(taller_client)
    # moc y los recursos de Qt (los menús)
    set_target_properties(taller_client PROPERTIES AUTOMOC ON AUTORCC ON)
    add_subdirectory(client)
    set_project_warnings(taller_client ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
    target_include_directories(taller_client PRIVATE ${libSDL2pp_SOURCE_DIR})
    target_link_libraries(taller_client taller_common SDL2pp::SDL2pp Qt6::Widgets)

    # Solo el menu en Qt, sin SDL: para ver las pantallas sin compilar el juego.
    include(client/menu/menu_sources.cmake)
    add_executable(taller_menu client/menu/main.cpp ${TALLER_MENU_SOURCES})
    set_target_properties(taller_menu PROPERTIES AUTOMOC ON AUTORCC ON)
    target_link_libraries(taller_menu taller_common Qt6::Widgets)
endif()

if(TALLER_SERVER)
    add_executable(taller_server)
    add_subdirectory(server)
    set_project_warnings(taller_server ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
    target_link_libraries(taller_server taller_common)
endif()

if(TALLER_EDITOR)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    add_executable(taller_editor)
    set_target_properties(taller_editor PROPERTIES AUTOMOC ON AUTORCC ON)
    add_subdirectory(editor)
    set_project_warnings(taller_editor ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
    target_link_libraries(taller_editor taller_common Qt6::Widgets)
endif()

# Testing section
# ---------------
# GoogleTest. Los tests compilan con taller_common y además con los .cpp del servidor y del cliente
# que prueban (listados en tests/CMakeLists.txt). Warnings un poco menos estrictos.
if(TALLER_TESTS)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(taller_tests)
    add_subdirectory(tests/)
    set_project_warnings(taller_tests ${TALLER_MAKE_WARNINGS_AS_ERRORS} TRUE)
    target_link_libraries(taller_tests taller_common GTest::gtest_main)
endif()

# Install section (install.sh)
# ---------------
# Binarios en ~/.local/bin con el nombre del juego, Assets/ y maps/ en ~/.local/share/TALLER_NAME y
# las configs en ~/.config/TALLER_NAME. Las configs que ya están no se pisan: pueden tener cambios
# del usuario.
if(TALLER_INSTALL)
    foreach(program server client editor)
        set(target taller_${program})
        if(TARGET ${target})
            install(PROGRAMS $<TARGET_FILE:${target}> DESTINATION bin
                    RENAME ${TALLER_NAME}-${program})
        endif()
    endforeach()
    install(DIRECTORY Assets maps DESTINATION share/${TALLER_NAME})
    foreach(config server_config.toml client_config.toml)
        install(CODE "
            if(EXISTS \"${TALLER_CONFIG_DIR}/${config}\")
                message(STATUS \"Kept: ${TALLER_CONFIG_DIR}/${config}\")
            else()
                file(INSTALL \"${CMAKE_CURRENT_SOURCE_DIR}/${config}\"
                     DESTINATION \"${TALLER_CONFIG_DIR}\")
            endif()
        ")
    endforeach()
endif()
