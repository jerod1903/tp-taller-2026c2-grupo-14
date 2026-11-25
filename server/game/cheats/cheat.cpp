#include "server/game/cheats/cheat.h"

#include <charconv>
#include <map>

#include "common/game_dto.h"
#include "server/game/characters/player.h"
#include "server/game/cheats/add_gold_cheat.h"
#include "server/game/cheats/finish_cheat.h"
#include "server/game/cheats/lose_gold_cheat.h"
#include "server/game/cheats/seeds_cheat.h"
#include "server/game/cheats/sleep_cheat.h"
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
    static const AddGoldCheat add_gold;
    static const LoseGoldCheat lose_gold;
    static const SleepCheat sleep;
    static const FinishCheat finish;
    static const std::map<std::string, const Cheat*> cheats = {
            {"seeds", &seeds}, {"add_gold", &add_gold}, {"lose_gold", &lose_gold},
            {"sleep", &sleep}, {"finish", &finish},
    };
    auto it = cheats.find(name);
    return it != cheats.end() ? it->second : nullptr;
}
