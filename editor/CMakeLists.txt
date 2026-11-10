target_sources(taller_editor
    PRIVATE
    # .cpp files
    main.cpp
    Areas.cpp
    BlockPalette.cpp
    MapCanvas.cpp
    EditorWindow.cpp
    ElementHelper.cpp
    Elements.cpp
    MapEditing.cpp
    MapHelper.cpp
    MapEditorScreen.cpp
    Mine.cpp
    MineHelper.cpp
    Passages.cpp
    SizeDialog.cpp
    SpriteSheets.cpp
    tools/AreaTool.cpp
    tools/EntranceTool.cpp
    tools/PaintTool.cpp
    tools/PassageTool.cpp
    tools/PlaceAreaTool.cpp
    tools/PlaceTool.cpp
    tools/SelectTool.cpp
    # El estilo de madera del menu del juego, tal como esta en client/menu
    ${CMAKE_SOURCE_DIR}/client/menu/components/MenuButton.cpp
    ${CMAKE_SOURCE_DIR}/client/menu/styles/StyleManager.cpp
    ${CMAKE_SOURCE_DIR}/client/menu/resources/menu.qrc
    PUBLIC
    # .h files
    Areas.h
    BlockPalette.h
    MapCanvas.h
    EditorWindow.h
    ElementHelper.h
    Elements.h
    MapEditing.h
    MapHelper.h
    MapEditorScreen.h
    Mine.h
    MineHelper.h
    Passages.h
    SizeDialog.h
    SpriteSheets.h
    tools/AreaTool.h
    tools/EntranceTool.h
    tools/Ghost.h
    tools/PaintTool.h
    tools/PassageTool.h
    tools/PlaceAreaTool.h
    tools/PlaceTool.h
    tools/SelectTool.h
    tools/Tool.h
    )

# Los sprites del mapa se leen de la carpeta de assets del repo
target_compile_definitions(taller_editor PRIVATE TALLER_ASSETS_DIR="${CMAKE_SOURCE_DIR}/Assets")
