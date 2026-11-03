target_sources(taller_server
    PRIVATE
    # .cpp files
    main.cpp
    server.cpp
    game/game.cpp
    game/game_registry.cpp
    lobby/lobby_session.cpp
    net/acceptor.cpp
    net/receiver.cpp
    net/sender.cpp
    net/server_protocol.cpp
    persistence/in_memory_game_store.cpp
    PUBLIC
    # .h files
    consts.h
    server.h
    game/game_event.h
    game/game.h
    game/game_registry.h
    lobby/lobby_session.h
    net/acceptor.h
    net/receiver.h
    net/sender.h
    net/server_protocol.h
    persistence/game_store.h
    persistence/in_memory_game_store.h
    )
