include(menu/menu_sources.cmake)
target_sources(taller_client
    PRIVATE
    # Menu en Qt: lista compartida con taller_menu (ver menu/menu_sources.cmake)
    ${TALLER_MENU_SOURCES}
    # .cpp files
    main.cpp
    network/ClientProtocol.cpp
    gui/prueba.cpp
    gui/cliente.cpp
    gui/interfaz.cpp
    gui/personaje.cpp
    gui/calculador_camara.cpp
    network/enviador.cpp
    network/recibidor.cpp
    PUBLIC
    # .h files
    network/ClientProtocol.h
    gui/cliente.h
    gui/interfaz.h
    gui/personaje.h
    gui/calculador_camara.h
    gui/constants.h
    network/enviador.h
    network/recibidor.h
    )
