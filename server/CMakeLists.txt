target_sources(taller_server
    PRIVATE
    # .cpp files
    acceptor.cpp
    lobby_session.cpp
    main.cpp
    server.cpp
    server_protocol.cpp
    PUBLIC
    # .h files
    acceptor.h
    lobby_session.h
    server.h
    consts.h
    server_protocol.h
    )
