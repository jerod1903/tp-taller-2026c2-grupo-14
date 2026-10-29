target_sources(taller_tests
    PRIVATE
    # .cpp files
    game_registry_test.cpp
    lobby_session_test.cpp
    logger_test.cpp
    server_protocol_test.cpp
    # Código del servidor bajo test (sin main.cpp)
    ../server/game/game_registry.cpp
    ../server/lobby/lobby_session.cpp
    ../server/net/server_protocol.cpp
    ../server/persistence/in_memory_game_store.cpp
    PUBLIC
    # .h files
    )
