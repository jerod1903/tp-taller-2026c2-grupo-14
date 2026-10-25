target_sources(taller_tests
    PRIVATE
    # .cpp files
    game_registry_test.cpp
    lobby_session_test.cpp
    logger_test.cpp
    # Código del servidor bajo test (sin main.cpp)
    ../server/game_registry.cpp
    ../server/in_memory_game_store.cpp
    ../server/lobby_session.cpp
    ../server/server_protocol.cpp
    PUBLIC
    # .h files
    )
