target_sources(taller_server
    PRIVATE
    # .cpp files
    acceptor.cpp
    game_registry.cpp
    in_memory_game_store.cpp
    lobby_session.cpp
    main.cpp
    server.cpp
    server_protocol.cpp
    PUBLIC
    # .h files
    acceptor.h
    game_registry.h
    game_store.h
    in_memory_game_store.h
    lobby_session.h
    server.h
    consts.h
    server_protocol.h
    )
