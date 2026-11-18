#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <cstdint>
#include <string>

// Lo que se ajusta del juego sin recompilar. Se lee de server_config.toml, en el root del repo.
struct ServerConfig {
    struct Game {
        uint8_t max_players;
        int tick_rate;  // ticks por segundo
        unsigned int outbox_size;
        float day_speed;  // segundos que dura un día
    };
    struct Player {
        int health;
        int energy;
        int gold;
        float speed;  // celdas por segundo
        // Los objetos iguales se apilan en un slot del inventario hasta esta cantidad.
        uint8_t max_stack;
    };
    // Lo que cansa cada acción.
    struct Energy {
        int dig;
        int water;
        int plant;
        int harvest;
        int milk;
        int attack;
        int mine;
        int talk;
    };
    struct Species {
        int price;            // lo que cuesta el animal
        int product_price;    // lo que se paga por la leche o el huevo
        int max_per_farm;
        int production_days;  // cada cuántos días da su producto
    };
    struct Animals {
        int cows;
        int chickens;
        // Donde arranca la fila de animales, respecto de donde aparece el jugador
        int offset_x;
        int offset_y;
        int health;
        float speed;     // celdas por segundo
        float min_walk;  // segundos
        float max_walk;
        // Hasta dónde llega el jugador para ordeñar: de su centro al de la vaca, en casillas.
        float milking_reach;
        Species cow;
        Species chicken;
    };

    // Lo que comparten los enemigos que pelean cuerpo a cuerpo
    struct Fighter {
        int health;
        int damage;      // vida que le saca a quien golpea
        float speed;     // celdas por segundo
        float sight;     // hasta donde ve a un jugador, en celdas
        float reach;     // hasta donde llega su golpe: de su centro al del jugador, en celdas
        float cooldown;  // segundos entre golpe y golpe
    };
    // Tiene que estar antes de Enemies, que la usa
    struct Skeleton {
        int count;  // cuantos aparecen en cada nivel
        Fighter fighter;
        float revive_seconds;  // cuanto tarda en revivir despues de morir
    };
    struct Enemies {
        // Al aparecer, ninguno queda a menos de esta distancia (en celdas) de un portal del nivel:
        // el que entra no se los encuentra encima.
        float spawn_distance;
        Skeleton skeleton;
    };

    Game game;
    Player player;
    Energy energy;
    Animals animals;
    Enemies enemies;
};

// Lanza std::runtime_error si el archivo no se puede leer, le falta un valor o uno es inválido.
ServerConfig load_server_config(const std::string& path);

// La de server_config.toml, que se lee una sola vez.
const ServerConfig& server_config();

#endif  // SERVER_CONFIG_H
