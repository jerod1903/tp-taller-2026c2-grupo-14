target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    menu/MainWindow.cpp
    menu/backend/MockLobbyProtocol.cpp
    menu/components/MenuButton.cpp
    menu/navigation/ScreenManager.cpp
    menu/screens/BaseScreen.cpp
    menu/screens/MainMenuScreen.cpp
    menu/screens/PlaceholderScreen.cpp
    menu/styles/StyleManager.cpp
    PUBLIC
    # .h files
    menu/MainWindow.h
    menu/backend/ILobbyProtocol.h
    menu/backend/LobbyTypes.h
    menu/backend/MockLobbyProtocol.h
    menu/components/MenuButton.h
    menu/navigation/ScreenManager.h
    menu/screens/BaseScreen.h
    menu/screens/MainMenuScreen.h
    menu/screens/PlaceholderScreen.h
    menu/styles/StyleManager.h
    )
