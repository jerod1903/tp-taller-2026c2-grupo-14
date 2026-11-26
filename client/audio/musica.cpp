#include "client/audio/musica.h"

#include <string>

#include "client/audio/constantes_musica.h"

std::string musica_de(const std::string& area, bool es_mina) {
    if (es_mina) {
        return MUSICA_DE_MINA;
    }
    if (MUSICA_DE_AREAS.contains(area)) {
        return MUSICA_DE_AREAS.at(area);
    }
    return MUSICA_POR_DEFECTO;
}
