#include "client/audio/musica.h"

#include <string>

#include "client/audio/constantes_musica.h"

Musica musica_de(const std::string& area, bool es_mina) {
    if (es_mina) {
        return Musica::Caverna;
    }
    auto interior = MUSICA_DE_INTERIORES.find(area);
    return interior != MUSICA_DE_INTERIORES.end() ? interior->second : Musica::GranjaYPueblo;
}
