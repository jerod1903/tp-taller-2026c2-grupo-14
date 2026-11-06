include(menu/menu_sources.cmake)
target_sources(taller_client
    PRIVATE
    # Menu en Qt: lista compartida con taller_menu (ver menu/menu_sources.cmake)
    ${TALLER_MENU_SOURCES}
    # .cpp files
    main.cpp
    network/ClientProtocol.cpp
    main_juego.cpp
    client.cpp
    game/game_loop.cpp
    gui/interfaz.cpp
    gui/personaje.cpp
    gui/calculador_camara.cpp
    gui/mapa.cpp
    network/enviador.cpp
    network/recibidor.cpp
    audio/reproductor_sonidos.cpp
    PUBLIC
    # .h files
    network/ClientProtocol.h
    client.h
    constants.h
    game/entrada.h
    game/estado_juego.h
    game/game_loop.h
    gui/interfaz.h
    gui/personaje.h
    gui/calculador_camara.h
    gui/mapa.h
    gui/rutas.h
    gui/sprite.h
    network/enviador.h
    network/recibidor.h
    audio/sonido.h
    audio/reproductor_sonidos.h
    )
