#ifndef REPRODUCTOR_SONIDOS_H
#define REPRODUCTOR_SONIDOS_H

#include <map>
#include <optional>

#include <SDL2pp/SDL2pp.hh>

#include "client/audio/sonido.h"

// Inicializa y cierra solo el audio de SDL: SDL2pp::SDL cerraria tambien el video al destruirse
class IniciadorAudio {
public:
    // Tira SDL2pp::Exception si no hay audio
    IniciadorAudio();
    ~IniciadorAudio();
    IniciadorAudio(const IniciadorAudio&) = delete;
    IniciadorAudio& operator=(const IniciadorAudio&) = delete;
};

// Carga todos los efectos al construirse y los reproduce sin bloquear: SDL_mixer los mezcla en su
// propio hilo, y si suenan varios a la vez los superpone. Si la maquina no tiene audio el juego
// sigue igual, sin sonido. Se construye despues de la Interfaz (que inicia SDL)
class ReproductorSonidos {
public:
    ReproductorSonidos();
    void reproducir(Sonido sonido);

private:
    // En orden: se destruyen al reves, primero los sonidos y al final el audio de SDL.
    // Vacios si no hay audio
    std::optional<IniciadorAudio> iniciador;
    std::optional<SDL2pp::SDLMixer> biblioteca;
    std::optional<SDL2pp::Mixer> mixer;
    std::map<Sonido, SDL2pp::Chunk> sonidos;
};

#endif
