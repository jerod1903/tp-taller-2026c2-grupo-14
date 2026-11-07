target_sources(taller_editor
    PRIVATE
    # .cpp files
    main.cpp
    MapCanvas.cpp
    EditorWindow.cpp
    MapEditorScreen.cpp
    Passages.cpp
    SpriteSheets.cpp
    # El estilo de madera del menu del juego, tal como esta en client/menu
    ${CMAKE_SOURCE_DIR}/client/menu/components/MenuButton.cpp
    ${CMAKE_SOURCE_DIR}/client/menu/styles/StyleManager.cpp
    ${CMAKE_SOURCE_DIR}/client/menu/resources/menu.qrc
    PUBLIC
    # .h files
    MapCanvas.h
    EditorWindow.h
    MapEditorScreen.h
    Passages.h
    SpriteSheets.h
    )

# Los sprites del mapa se leen de la carpeta de assets del repo
target_compile_definitions(taller_editor PRIVATE TALLER_ASSETS_DIR="${CMAKE_SOURCE_DIR}/Assets")
