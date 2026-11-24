#ifndef REPRODUCTOR_SONIDOS_H
#define REPRODUCTOR_SONIDOS_H

#include <map>
#include <optional>

#include <SDL2pp/SDL2pp.hh>

#include "client/audio/musica.h"
#include "client/audio/sonido.h"

class ReproductorSonidos {
public:
    ReproductorSonidos();
    ~ReproductorSonidos();
    void reproducir(Sonido sonido);
    void poner_musica(Musica musica);

private:
    void cerrar();
    void cargar_musicas();
    bool audio_iniciado = false;
    std::optional<SDL2pp::SDLMixer> biblioteca;
    std::optional<SDL2pp::Mixer> mixer;
    std::map<Sonido, SDL2pp::Chunk> sonidos;
    std::map<Musica, SDL2pp::Music> musicas;
    std::optional<Musica> sonando;
};

#endif
