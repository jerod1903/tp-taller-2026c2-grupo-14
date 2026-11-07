#ifndef REPRODUCTOR_SONIDOS_H
#define REPRODUCTOR_SONIDOS_H

#include <map>
#include <optional>

#include <SDL2pp/SDL2pp.hh>

#include "client/audio/sonido.h"

// Carga todos los efectos al construirse y los reproduce sin bloquear: SDL_mixer los mezcla en su
// propio hilo, y si suenan varios a la vez los superpone. Si la maquina no tiene audio el juego
// sigue igual, sin sonido. Su dueña es la Interfaz, que antes inicia SDL
class ReproductorSonidos {
public:
    ReproductorSonidos();
    ~ReproductorSonidos();
    void reproducir(Sonido sonido);

private:
    // libera todo en orden inverso al que se armo y al final cierra el audio de sdl
    void cerrar();
    bool audio_iniciado = false;
    // vacios si no hay audio
    std::optional<SDL2pp::SDLMixer> biblioteca;
    std::optional<SDL2pp::Mixer> mixer;
    std::map<Sonido, SDL2pp::Chunk> sonidos;
};

#endif
