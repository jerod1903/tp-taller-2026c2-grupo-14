#ifndef REPRODUCTOR_SONIDOS_H
#define REPRODUCTOR_SONIDOS_H

#include <map>
#include <optional>

#include <SDL2pp/SDL2pp.hh>

#include "client/audio/sonido.h"

class ReproductorSonidos {
public:
    ReproductorSonidos();
    ~ReproductorSonidos();
    void reproducir(Sonido sonido);

private:
    void cerrar();
    bool audio_iniciado = false;
    std::optional<SDL2pp::SDLMixer> biblioteca;
    std::optional<SDL2pp::Mixer> mixer;
    std::map<Sonido, SDL2pp::Chunk> sonidos;
};

#endif
