#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

#include "common/game_dto.h"
#include "common/lib/queue.h"
#include "common/lib/thread.h"
#include "server/net/server_protocol.h"

// Un comando de un jugador, listo para que lo procese la partida.
struct Action {
    uint32_t player_id;
    ClientMessage message;
};

// Solo se declara: client_handler.h incluye este header (necesita Action).
class ClientHandler;

// Una partida en ejecución: un hilo con su gameloop.
class Game: public Thread {
public:
    // Definidos en game.cpp, donde ClientHandler ya está completo.
    Game();
    ~Game() override;

    // Crea y arranca el ClientHandler del jugador, y devuelve su player_id.
    uint32_t add_player(ServerProtocol&& protocol);
    void remove_player(uint32_t player_id);

    // Cada tick: sacar jugadores desconectados, try_pop de las acciones,
    // aplicarlas y mandar el estado.
    void run() override;
    
    void start() override;

    // Corta el loop y cierra la cola de acciones.
    void stop() override;

private:
    static constexpr uint32_t INITIAL_LIFE = 100;    // valores de ejemplo
    static constexpr uint32_t INITIAL_ENERGY = 100;
    static constexpr int TICK_MS = 33;

    struct PlayerSlot {
        PlayerState state;
        std::unique_ptr<ClientHandler> client;
    };

    Queue<Action> inbox;
    std::map<uint32_t, PlayerSlot> players;
    uint32_t next_player_id = 1;
    std::mutex mutex;

    // Solo se joinea un thread que efectivamente arrancó.
    bool started = false;

    void remove_disconnected();
    void process_actions();
    void broadcast_state();

    // Por sobrecarga
    void apply(uint32_t player_id, const MoveCommand& move);
    void apply(uint32_t player_id, const PickCommand& pick);
  
};

#endif  // GAME_H