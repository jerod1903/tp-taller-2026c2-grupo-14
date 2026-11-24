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
        cargar_musicas();
    } catch (const SDL2pp::Exception& e) {
        std::cerr << MENSAJE_SIN_SONIDO << e.what() << std::endl;
        cerrar();
    }
}

ReproductorSonidos::~ReproductorSonidos() { cerrar(); }

void ReproductorSonidos::cerrar() {
    sonidos.clear();
    musicas.clear();
    sonando.reset();
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

void ReproductorSonidos::cargar_musicas() {
    for (const auto& [musica, ruta]: RUTAS_MUSICAS) {
        try {
            musicas.emplace(musica, SDL2pp::Music(ruta));
        } catch (const SDL2pp::Exception& e) {
            std::cerr << MENSAJE_SIN_MUSICA << e.what() << std::endl;
        }
    }
    mixer->SetMusicVolume(client_config().audio.volumen_musica);
}

void ReproductorSonidos::poner_musica(Musica musica) {
    if (!mixer || sonando == musica) {
        return;
    }
    sonando = musica;
    auto encontrada = musicas.find(musica);
    if (encontrada == musicas.end()) {
        mixer->HaltMusic();
        return;
    }
    try {
        mixer->FadeInMusic(encontrada->second, -1, static_cast<int>(FUNDIDO_MUSICA.count()));
    } catch (const SDL2pp::Exception&) {}
}
