#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <cstdint>
#include <string>

#include "common/farm_dto.h"

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
        float speed;         // celdas por segundo
        float tired_energy;  // fracción de la energía desde la que camina a mitad de velocidad
        float bad_night_recovery;  // fracción de vida y energía al despertar si no durmió en su
                                   // cama
        int sword_damage;          // vida que le saca a un enemigo cada golpe de espada
        float attack_cooldown;     // segundos entre golpe y golpe
        float weak_health;         // fracción de la vida desde la que pega menos
        float weak_damage;         // fracción de su daño que pega cuando está así de débil
    };
    // Vale para todos los inventarios: el del jugador y los cofres.
    struct Inventory {
        // Los objetos iguales se apilan en un slot hasta esta cantidad.
        uint8_t max_stack;
    };
    // Lo que cansa cada acción.
    struct Energy {
        int dig;
        int water;
        int plant;
        int harvest;
        int milk;
        int pick_up;
        int attack;
        int mine;
        int talk;
    };
    struct Species {
        int price;          // lo que cuesta el animal
        int product_price;  // lo que se paga por la leche o el huevo
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
    // Una pareja: la vampira, más rápida (fast_speed), y el vampiro, que pega más fuerte
    // (strong_damage). Los demás valores, en `fighter`, son de los dos.
    struct Vampire {
        int pairs;  // cuantas parejas aparecen en cada nivel
        Fighter fighter;
        float fast_speed;
        int strong_damage;
        float revive_seconds;  // muerto, cuanto tarda en revivir despues de que cae sangre cerca
        float blood_reach;     // hasta donde (en celdas) le llega la sangre de un jugador
    };
    // Un grupo de entre min_group y max_group; `fighter` vale para cada uno.
    struct Orc {
        int min_group;
        int max_group;
        Fighter fighter;
    };
    // No se la puede matar: tira llamas de un color, sorteado al aparecer (verde con green_chance).
    struct Skull {
        float speed;
        float sight;
        float cooldown;  // segundos entre llama y llama
        float green_chance;
    };
    // Las llamas azules; las verdes hacen el doble de daño y van a la mitad de velocidad.
    struct Flame {
        int damage;
        float speed;  // celdas por segundo
        float range;  // celdas que recorre antes de apagarse
    };
    struct Enemies {
        // Al aparecer, ninguno queda a menos de esta distancia (en celdas) de un portal del nivel:
        // el que entra no se los encuentra encima.
        float spawn_distance;
        // Sin nadie a la vista deambulan como los animales: cambian de dirección cada tanto
        float min_walk;  // segundos
        float max_walk;
        Skeleton skeleton;
        Vampire vampire;
        Orc orc;
        Skull skull;
        Flame flame;
    };
    // Una trampa: active_time segundos activa y idle_time inactiva, una y otra vez. Activa, le
    // saca `damage` a cada jugador en su zona, una vez por activación.
    struct Trap {
        int damage;
        float active_time;
        float idle_time;
    };
    struct Flamethrower {
        Trap trap;
        int reach;  // celdas que alcanza el chorro, contando la suya
    };
    // Cuántas trampas de cada una aparecen en cada nivel de la mazmorra, al azar
    struct Traps {
        int spikes_count;
        int flamethrowers_count;
        Trap spikes;
        Flamethrower flamethrower;
    };
    // Los comerciantes. Los precios de los animales, sus productos y las semillas y cosechas están
    // en [animals.*] y [crops.*]; lo que cansa hablarles, en [energy].
    struct Market {
        float reach;  // casillas, del centro del jugador al de la celda del comerciante
        // Lo que paga Marnie por un animal que le vendés; no puede superar su price.
        int cow_buyback;
        int chicken_buyback;
    };

    struct CropSpec {
        int seed_price;  // lo que cuesta la semilla
        int base_price;  // lo que se paga por la cosecha de calidad base
        int waterings_per_day;
        int days_to_grow;         // desde que se planta hasta que se puede cosechar
        int regrow_days;          // después de cosechar vuelve a dar en estos días; 0 si no
        int skippable_waterings;  // riegos que se pueden saltear en un día sin que deje de crecer
    };
    // Cómo se calcula la calidad de una cosecha: un puntaje de 0 a 1 por qué tan rápido se regó y
    // qué tan rápido se cosechó, por un multiplicador de suerte al azar.
    struct Quality {
        float watering_weight;     // cuánto pesa el riego en el puntaje; lo demás, la cosecha
        float harvest_grace_days;  // días lista sin cosechar hasta que esa parte llega a 0
        float luck_min;
        float luck_max;
        // Puntaje mínimo de cada calidad; por debajo de good, base
        float good;
        float excellent;
        float incredible;
        float perfect;
    };
    struct Crops {
        CropSpec strawberry;
        CropSpec scallion;
        CropSpec potato;
        CropSpec pumpkin;
        Quality quality;

        // Lanza std::invalid_argument con CropType::None.
        const CropSpec& of(CropType type) const;
    };

    struct OreSpec {
        int hits;   // golpes de pico para sacarlo
        int price;  // lo que paga Clint
    };

    struct Mining {
        int ores_per_level;
        float lapis_break_chance;
        float silver_orc_chance;
        int gold_min_depth;  // profundidad mínima del oro; el nivel de entrada es la 0
        OreSpec coal;
        OreSpec lapis;
        OreSpec silver;
        OreSpec gold;

        const OreSpec& of(ItemType ore) const;
    };


    Game game;
    Player player;
    Inventory inventory;
    Energy energy;
    Animals animals;
    Enemies enemies;
    Traps traps;
    Crops crops;
    struct Cheats {
        int seeds;  // cuántas de cada tipo da "$seeds" sin cantidad
        int gold;   // cuánto da "$gold" sin cantidad
    } cheats;
    Market market;
    Mining mining;
};

// Lanza std::runtime_error si el archivo no se puede leer, le falta un valor o uno es inválido.
ServerConfig load_server_config(const std::string& path);

// La de server_config.toml, que se lee una sola vez.
const ServerConfig& server_config();

#endif  // SERVER_CONFIG_H
