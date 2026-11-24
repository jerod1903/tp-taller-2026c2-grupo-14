#ifndef MUSICA_H
#define MUSICA_H

#include <cstdint>
#include <string>

enum class Musica : uint8_t { Cabana, GranjaYPueblo, Mercado, Caverna };

Musica musica_de(const std::string& area, bool es_mina);

#endif
