#include "reproductor_sonidos.h"

#include <iostream>

#include "client/constants.h"
#include "client/gui/rutas.h"

IniciadorAudio::IniciadorAudio() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        throw SDL2pp::Exception("SDL_InitSubSystem");
    }
}

IniciadorAudio::~IniciadorAudio() { SDL_QuitSubSystem(SDL_INIT_AUDIO); }

ReproductorSonidos::ReproductorSonidos() {
    try {
        iniciador.emplace();
        biblioteca.emplace(MIX_INIT_OGG);
        mixer.emplace(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS,
                      TAMANO_BUFFER_AUDIO);
        mixer->AllocateChannels(CANALES_AUDIO);
        for (const auto& [sonido, ruta]: RUTAS_SONIDOS) {
            sonidos.emplace(sonido, SDL2pp::Chunk(ruta));
        }
    } catch (const SDL2pp::Exception& e) {
        std::cerr << "Sin sonido: " << e.what() << std::endl;
        sonidos.clear();
        mixer.reset();
        biblioteca.reset();
        iniciador.reset();
    }
}

void ReproductorSonidos::reproducir(Sonido sonido) {
    auto encontrado = sonidos.find(sonido);
    if (!mixer || encontrado == sonidos.end()) {
        return;
    }
    try {
        mixer->PlayChannel(-1, encontrado->second);
    } catch (const SDL2pp::Exception&) {
        // Todos los canales ocupados: este sonido se pierde, no vale la pena cortar otro
    }
}
