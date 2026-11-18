#include "server/game/clicks/click_action.h"

#include "server/game/clicks/use_tool.h"

// Va última: lo que haya en la celda (un comerciante, otro jugador) gana sobre la herramienta.
const std::vector<const ClickAction*>& click_actions() {
    static const UseTool use_tool;
    static const std::vector<const ClickAction*> all = {&use_tool};
    return all;
}
