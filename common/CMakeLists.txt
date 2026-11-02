target_sources(taller_common
    PRIVATE
    # .cpp files
    base_protocol.cpp
    logger.cpp
    net/liberror.cpp
    net/resolver.cpp
    net/resolvererror.cpp
    net/socket.cpp
    PUBLIC
    # .h files
    base_protocol.h
    estado_personaje.h
    lobby_dto.h
    logger.h
    lib/queue.h
    lib/thread.h
    net/liberror.h
    net/resolver.h
    net/resolvererror.h
    net/socket.h
    operations.h
    requests.h
    )
