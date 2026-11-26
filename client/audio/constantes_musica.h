#ifndef CONSTANTES_MUSICA_H
#define CONSTANTES_MUSICA_H

#include <map>
#include <string>

#include "client/gui/rutas.h"

inline const char* const MUSICA_POR_DEFECTO = RUTA_MUSICA_GRANJA_PUEBLO;
inline const char* const MUSICA_DE_MINA = RUTA_MUSICA_CAVERNA;
inline const std::map<std::string, const char*> MUSICA_DE_AREAS = {
        {"cabin", RUTA_MUSICA_GRANJA_PUEBLO},
        {"market", RUTA_MUSICA_MERCADO},
        {"community_center", RUTA_MUSICA_MERCADO},
};

#endif
