include(menu/menu_sources.cmake)
target_sources(taller_client
    PRIVATE
    # Menu en Qt: lista compartida con taller_menu (ver menu/menu_sources.cmake)
    ${TALLER_MENU_SOURCES}
    # .cpp files
    main.cpp
    network/ClientProtocol.cpp
    gui/prueba.cpp
    gui/interfaz_juego.cpp
    gui/caverna.cpp
    gui/personaje.cpp
    gui/calculador_camara.cpp
    PUBLIC
    # .h files
    network/ClientProtocol.h
    gui/interfaz_juego.h
    gui/caverna.h
    gui/personaje.h
    gui/calculador_camara.h
    gui/constants.h
    )
