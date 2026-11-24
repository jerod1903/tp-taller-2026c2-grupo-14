#ifndef CONSTANTES_MUSICA_H
#define CONSTANTES_MUSICA_H

#include <map>
#include <string>

#include "client/audio/musica.h"

inline const std::map<std::string, Musica> MUSICA_DE_INTERIORES = {
        {"cabin", Musica::Cabana},
        {"market", Musica::Mercado},
        {"community_center", Musica::Mercado},
};

#endif
