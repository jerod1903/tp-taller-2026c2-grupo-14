target_sources(taller_tests
    PRIVATE
    # .cpp files
    lobby_session_test.cpp
    logger_test.cpp
    # Código del servidor bajo test (sin main.cpp)
    ../server/lobby_session.cpp
    ../server/server_protocol.cpp
    PUBLIC
    # .h files
    )
