#ifndef CLIENT_H
#define CLIENT_H

#include <optional>

#include "client/app/IGameView.h"
#include "client/network/IGameProtocol.h"

class Cliente: public IGameView {
public:
    explicit Cliente(IGameProtocol& protocolo);
    std::optional<EndReason> run(const GameSession& session) override;

private:
    IGameProtocol& protocolo;
};

#endif
