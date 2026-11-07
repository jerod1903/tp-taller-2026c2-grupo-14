target_sources(taller_editor
    PRIVATE
    # .cpp files
    main.cpp
    EditorWindow.cpp
    # El estilo de madera del menu del juego, tal como esta en client/menu
    ${CMAKE_SOURCE_DIR}/client/menu/components/MenuButton.cpp
    ${CMAKE_SOURCE_DIR}/client/menu/styles/StyleManager.cpp
    ${CMAKE_SOURCE_DIR}/client/menu/resources/menu.qrc
    PUBLIC
    # .h files
    EditorWindow.h
    )
