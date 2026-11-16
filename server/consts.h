#ifndef SERVER_CONSTANTS_H
#define SERVER_CONSTANTS_H

#include <cstddef>
#include <cstdint>

class ServerConstants {
public:
    static constexpr int EXIT_OK = 0;
    static constexpr int EXIT_ERROR = 1;

    static constexpr int EXPECTED_ARGC = 2;
    static constexpr int ARG_PROGRAM_NAME = 0;
    static constexpr int ARG_SERVICENAME = 1;
    static constexpr const char* USAGE_PREFIX = "Uso: ";
    static constexpr const char* USAGE_ARGS = " <servicename>\n";
    static constexpr const char* ERROR_PREFIX = "Error: ";
    static constexpr const char* UNKNOWN_ERROR = "Error desconocido";

    static constexpr const char* EXIT_COMMAND = "q";

    static constexpr std::size_t MAX_NAME_LEN = 20;
    static constexpr std::size_t MAX_LOGGED_LEN = 2 * MAX_NAME_LEN;

    static constexpr const char* MAP_REQUIREMENTS = TALLER_MAPS_DIR "/config/map_requirements.toml";

    // Chat: "@nombre texto" es un mensaje privado para ese personaje.
    static constexpr char PRIVATE_CHAT_PREFIX = '@';
    // Aviso al que mando un privado a alguien que no esta: "<nombre> no está conectado".
    static constexpr const char* NOT_CONNECTED = " no está conectado";
};

#endif  // SERVER_CONSTANTS_H
