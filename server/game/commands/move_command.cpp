#include "server/game/commands/move_command.h"

#include <map>
#include <utility>

namespace {
// (signo de dirX, signo de dirY) -> dirección
const std::map<std::pair<int, int>, Direction> DIRECTIONS = {
        {{0, 0}, Direction::NONE},       {{0, -1}, Direction::UP},
        {{0, 1}, Direction::DOWN},       {{-1, 0}, Direction::LEFT},
        {{1, 0}, Direction::RIGHT},      {{-1, -1}, Direction::UP_LEFT},
        {{1, -1}, Direction::UP_RIGHT},  {{-1, 1}, Direction::DOWN_LEFT},
        {{1, 1}, Direction::DOWN_RIGHT},
};

int sign(int8_t value) { return (value > 0) - (value < 0); }

Direction direction_of(const MoveRequest& request) {
    return DIRECTIONS.at({sign(request.dirX), sign(request.dirY)});
}
}  // namespace

MoveCommand::MoveCommand(uint32_t player_id, const MoveRequest& request):
        GameCommand(player_id), request(request) {}

void MoveCommand::execute(GameContext& context) {
    Player* player = context.find_player(player_id);
    if (player) {
        player->set_direction(direction_of(request));
    }
}
