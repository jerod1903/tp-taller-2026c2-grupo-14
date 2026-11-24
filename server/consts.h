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

    // Cantidad de slots del inventario de un jugador.
    static constexpr uint8_t PLAYER_INVENTORY_SIZE = 9;

    // Cantidad de slots del inventario de un cofre.
    static constexpr uint8_t CHEST_SIZE = 30;

    static constexpr const char* MAP_REQUIREMENTS = TALLER_MAPS_DIR "/config/map_requirements.toml";

    // Chat: "@nombre texto" es un mensaje privado para ese personaje.
    static constexpr char PRIVATE_CHAT_PREFIX = '@';
    // Chat: "$nombre [payload]" activa ese cheat; no le llega a nadie más.
    static constexpr char CHEAT_PREFIX = '$';
    // Aviso al que escribió un cheat que no existe: "Cheat desconocido: <nombre>".
    static constexpr const char* UNKNOWN_CHEAT = "Cheat desconocido: ";
    // Aviso al que mando un privado a alguien que no esta: "<nombre> no está conectado".
    static constexpr const char* NOT_CONNECTED = " no está conectado";
    // Chat: "/comando ..." es para el comerciante con el que habla el jugador.
    static constexpr char MERCHANT_COMMAND_PREFIX = '/';
    // Respuestas a GameOp::Invite, por el chat como mensaje de sistema.
    static constexpr const char* INVITE_CODE = "Código para invitar: ";
    static constexpr const char* INVITE_NOT_OWNER = "Solo quien creó la partida puede invitar";
    static constexpr const char* INVITE_FAILED = "No se pudo generar el código de invitación";
};

#endif  // SERVER_CONSTANTS_H
