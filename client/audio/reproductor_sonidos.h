#ifndef REPRODUCTOR_SONIDOS_H
#define REPRODUCTOR_SONIDOS_H

#include <map>
#include <optional>
#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "client/audio/sonido.h"

class ReproductorSonidos {
public:
    ReproductorSonidos();
    ~ReproductorSonidos();
    void reproducir(Sonido sonido);
    void poner_musica(const std::string& ruta);
    void alternar_musica();
    bool musica_apagada() const;

private:
    void cerrar();
    void cargar_musicas();
    bool audio_iniciado = false;
    std::optional<SDL2pp::SDLMixer> biblioteca;
    std::optional<SDL2pp::Mixer> mixer;
    std::map<Sonido, SDL2pp::Chunk> sonidos;
    std::map<std::string, SDL2pp::Music> musicas;
    std::optional<std::string> sonando;
    bool apagada = false;
};

#endif
