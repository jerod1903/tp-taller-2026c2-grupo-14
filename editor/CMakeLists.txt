target_sources(taller_editor
    PRIVATE
    # .cpp files
    main.cpp
    BlockPalette.cpp
    MapCanvas.cpp
    EditorWindow.cpp
    Elements.cpp
    FloorSizeDialog.cpp
    MapEditing.cpp
    MapHelper.cpp
    MapEditorScreen.cpp
    Mine.cpp
    Passages.cpp
    SpriteSheets.cpp
    tools/EntranceTool.cpp
    tools/PaintTool.cpp
    tools/PassageTool.cpp
    tools/PlaceTool.cpp
    tools/SelectTool.cpp
    # El estilo de madera del menu del juego, tal como esta en client/menu
    ${CMAKE_SOURCE_DIR}/client/menu/components/MenuButton.cpp
    ${CMAKE_SOURCE_DIR}/client/menu/styles/StyleManager.cpp
    ${CMAKE_SOURCE_DIR}/client/menu/resources/menu.qrc
    PUBLIC
    # .h files
    BlockPalette.h
    MapCanvas.h
    EditorWindow.h
    Elements.h
    FloorSizeDialog.h
    MapEditing.h
    MapHelper.h
    MapEditorScreen.h
    Mine.h
    Passages.h
    SpriteSheets.h
    tools/EntranceTool.h
    tools/PaintTool.h
    tools/PassageTool.h
    tools/PlaceTool.h
    tools/SelectTool.h
    tools/Tool.h
    )

# Los sprites del mapa se leen de la carpeta de assets del repo
target_compile_definitions(taller_editor PRIVATE TALLER_ASSETS_DIR="${CMAKE_SOURCE_DIR}/Assets")
