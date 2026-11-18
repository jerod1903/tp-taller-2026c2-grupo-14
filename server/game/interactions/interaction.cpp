#include "server/game/interactions/interaction.h"

#include "server/game/interactions/milk.h"

const std::vector<const Interaction*>& interactions() {
    static const Milk milk;
    static const std::vector<const Interaction*> all = {&milk};
    return all;
}
