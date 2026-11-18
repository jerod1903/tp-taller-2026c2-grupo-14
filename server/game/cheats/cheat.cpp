#include "server/game/cheats/cheat.h"

#include <charconv>
#include <map>

#include "common/game_dto.h"
#include "server/game/characters/player.h"
#include "server/game/cheats/gold_cheat.h"
#include "server/game/cheats/seeds_cheat.h"
#include "server/game/commands/game_context.h"

std::optional<int> Cheat::amount(const std::string& payload, int fallback) {
    if (payload.empty()) {
        return fallback;
    }
    int value = 0;
    const char* end = payload.data() + payload.size();
    auto [parsed_until, error] = std::from_chars(payload.data(), end, value);
    if (error != std::errc() || parsed_until != end || value <= 0) {
        return std::nullopt;
    }
    return value;
}

void Cheat::tell(const Player& player, GameContext& context, const std::string& text) {
    context.sessions().send_to(player.get_id(), ChatMessage{ChatKind::System, "", text});
}

const Cheat* cheat_for(const std::string& name) {
    static const SeedsCheat seeds;
    static const GoldCheat gold;
    static const std::map<std::string, const Cheat*> cheats = {
            {"seeds", &seeds},
            {"gold", &gold},
    };
    auto it = cheats.find(name);
    return it != cheats.end() ? it->second : nullptr;
}
