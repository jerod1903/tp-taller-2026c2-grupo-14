#ifndef MUNDO_H
#define MUNDO_H

#include <string>

#include "common/map/MapDefinition.h"

// Lo que puede tener el id de un mundo: es el nombre de su archivo en la carpeta de mapas
inline const char* const CARACTERES_WORLD_ID =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";

// El mapa del mundo con ese id (maps/<id>.toml). Lo leen una sola vez el juego, para el área y el
// mapa general, y el menú, para la vista previa. Lanza si el id tiene otros caracteres o si el
// archivo no se puede leer.
stardew::maps::MapDefinition cargar_mundo(const std::string& world_id);

#endif
