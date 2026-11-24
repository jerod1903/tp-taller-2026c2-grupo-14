#ifndef ITEM_NAMES_H
#define ITEM_NAMES_H

#include <string>

#include "common/farm_dto.h"

// Cómo se llama ese objeto en los mensajes del chat ("semillas de papa", "carbón").
std::string item_name(ItemType item);

#endif  // ITEM_NAMES_H
