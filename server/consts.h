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

    // Hasta que exista la config TOML.
    static constexpr uint8_t MAX_PLAYERS = 4;

    static constexpr const char* MAP_REQUIREMENTS = TALLER_MAPS_DIR "/config/map_requirements.toml";

    static constexpr int TICK_RATE = 30;

    // Hasta que exista la config de juego (GameConfig).
    static constexpr int INITIAL_HEALTH = 100;
    static constexpr int INITIAL_ENERGY = 100;
    static constexpr int INITIAL_GOLD = 500;
    static constexpr float PLAYER_SPEED = 4.0f;  // celdas por segundo
    // Mensajes pendientes por jugador; con la outbox llena se descartan snapshots.
    static constexpr unsigned int OUTBOX_SIZE = 64;
};

#endif  // SERVER_CONSTANTS_H
