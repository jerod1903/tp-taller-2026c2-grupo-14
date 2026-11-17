#include "reproductor_sonidos.h"

#include <iostream>

#include <SDL2/SDL.h>

#include "client/config/client_config.h"
#include "client/constants.h"
#include "client/gui/rutas.h"

ReproductorSonidos::ReproductorSonidos() {
    try {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            throw SDL2pp::Exception(FUNCION_INICIO_AUDIO);
        }
        audio_iniciado = true;
        biblioteca.emplace(MIX_INIT_OGG);
        mixer.emplace(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS,
                      client_config().audio.tamano_buffer);
        mixer->AllocateChannels(client_config().audio.canales);
        for (const auto& [sonido, ruta]: RUTAS_SONIDOS) {
            sonidos.emplace(sonido, SDL2pp::Chunk(ruta));
        }
    } catch (const SDL2pp::Exception& e) {
        std::cerr << MENSAJE_SIN_SONIDO << e.what() << std::endl;
        cerrar();
    }
}

ReproductorSonidos::~ReproductorSonidos() { cerrar(); }

void ReproductorSonidos::cerrar() {
    sonidos.clear();
    mixer.reset();
    biblioteca.reset();
    if (audio_iniciado) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        audio_iniciado = false;
    }
}

void ReproductorSonidos::reproducir(Sonido sonido) {
    auto encontrado = sonidos.find(sonido);
    if (!mixer || encontrado == sonidos.end()) {
        return;
    }
    try {
        mixer->PlayChannel(-1, encontrado->second);
    } catch (const SDL2pp::Exception&) {}
}
