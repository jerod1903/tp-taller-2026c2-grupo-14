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

    // Animales de la granja. Provisorio: hasta que se compren a Marnie, la granja arranca con estos.
    static constexpr int INITIAL_COWS = 2;
    static constexpr int INITIAL_CHICKENS = 3;
    // Donde arranca la fila de animales, respecto de donde aparece el jugador
    static constexpr int ANIMALS_OFFSET_X = -2;
    static constexpr int ANIMALS_OFFSET_Y = 3;
    static constexpr int ANIMAL_HEALTH = 100;
    static constexpr float ANIMAL_SPEED = 1.0f;  // celdas por segundo
    // Cuanto camina (o se queda quieto) antes de elegir otra direccion, en segundos.
    static constexpr float ANIMAL_MIN_WALK = 1.0f;
    static constexpr float ANIMAL_MAX_WALK = 3.0f;
    // Provisorio, hasta que exista el ciclo de dias: el enunciado dice que la vaca da leche cada
    // 2 dias. En segundos de juego.
    static constexpr float COW_READY_SECONDS = 20.0f;
    // Hasta donde llega el jugador para ordeñar: de su centro al de la vaca, en casillas.
    static constexpr float MILKING_REACH = 1.5f;
    // Los objetos iguales se apilan en un slot del inventario hasta esta cantidad.
    static constexpr uint8_t MAX_STACK = 50;
    // Mensajes pendientes por jugador; con la outbox llena se descartan snapshots.
    static constexpr unsigned int OUTBOX_SIZE = 64;

    // Chat: "@nombre texto" es un mensaje privado para ese personaje.
    static constexpr char PRIVATE_CHAT_PREFIX = '@';
    // Aviso al que mando un privado a alguien que no esta: "<nombre> no está conectado".
    static constexpr const char* NOT_CONNECTED = " no está conectado";
};

#endif  // SERVER_CONSTANTS_H
