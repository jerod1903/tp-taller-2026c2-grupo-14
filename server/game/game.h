#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <mutex>

#include "common/game_dto.h"
#include "common/lib/queue.h"
#include "common/lib/thread.h"

// Un comando de un jugador, listo para que lo procese la partida.
struct Action {
    uint32_t player_id;
    ClientMessage message;
};

// Una partida en ejecución: un hilo con su gameloop.
class Game: public Thread {
public:
    // Los ServerReceiver hacen push aca (muchos productores).
    Queue<Action>& actions();

    // Registra la cola de salida de un jugador y devuelve su player_id.
    uint32_t add_player(Queue<ServerMessage>& outbox);
    void remove_player(uint32_t player_id);

    // Cada tick: try_pop de todas las acciones, aplicarlas y mandar el estado.
    void run() override;

    // Corta el loop y cierra la cola de acciones.
    void stop() override;

private:
    static constexpr uint32_t INITIAL_LIFE = 100;    // valores de ejemplo
    static constexpr uint32_t INITIAL_ENERGY = 100;  
    static constexpr int TICK_MS = 33;

    struct PlayerSlot {
        PlayerState state;
        Queue<ServerMessage>* outbox;
    };

    Queue<Action> inbox;
    std::map<uint32_t, PlayerSlot> players;
    uint32_t next_player_id = 1;
    std::mutex mutex;

    void process_actions();
    void apply(uint32_t player_id, const MoveCommand& move);
    void broadcast_state();
};

#endif  // GAME_H