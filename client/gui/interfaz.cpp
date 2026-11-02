#include "interfaz.h"

#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include <SDL2/SDL_image.h>

#include "constants.h"

Interfaz::Interfaz(Enviador& enviador, Recibidor& recibidor):
        enviador(enviador),
        recibidor(recibidor),
        sdl(SDL_INIT_VIDEO),
        sdl_image(IMG_INIT_PNG),
        ventana("Jueguito", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ANCHO_VENTANA,
                ALTO_VENTANA, SDL_WINDOW_SHOWN),
        renderer(ventana, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        sprite_personaje(renderer, RUTA_SPRITE_PERSONAJE),
        calculador_camara(ANCHO_VENTANA, ALTO_VENTANA) {}

void Interfaz::ejecutar() {
    while (corriendo) {
        Uint32 inicio = SDL_GetTicks();
        procesar_eventos();
        actualizar();
        dibujar();
        Uint32 duracion = SDL_GetTicks() - inicio;
        if (duracion < MS_POR_FRAME) {
            SDL_Delay(MS_POR_FRAME - duracion);
        }
    }
}

void Interfaz::procesar_eventos() {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        if (evento.type == SDL_QUIT ||
            (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_ESCAPE)) {
            corriendo = false;
        }
    }
}

void Interfaz::actualizar() {
    static std::map<std::pair<int, int>, std::pair<SDL_Scancode, SDL_Scancode>> teclas_direccion = {
            {{-1, 0}, {SDL_SCANCODE_LEFT, SDL_SCANCODE_A}},
            {{1, 0}, {SDL_SCANCODE_RIGHT, SDL_SCANCODE_D}},
            {{0, -1}, {SDL_SCANCODE_UP, SDL_SCANCODE_W}},
            {{0, 1}, {SDL_SCANCODE_DOWN, SDL_SCANCODE_S}}};
    const Uint8* teclas = SDL_GetKeyboardState(nullptr);
    int dir_x = 0;
    int dir_y = 0;
    for (const auto& [direccion, par_teclas]: teclas_direccion) {
        if (teclas[par_teclas.first] || teclas[par_teclas.second]) {
            dir_x += direccion.first;
            dir_y += direccion.second;
        }
    }
    if (dir_x != 0 || dir_y != 0) {
        Request request{};
        request.tipo = Operation::Move;
        request.dir_x = static_cast<int8_t>(dir_x);
        request.dir_y = static_cast<int8_t>(dir_y);
        enviador.enviar(request);
    }
    std::optional<std::vector<EstadoPersonaje>> estado = recibidor.recibir();
    if (estado) {
        actualizar_personajes(*estado);
    }
}

void Interfaz::actualizar_personajes(const std::vector<EstadoPersonaje>& estado) {
    std::set<uint16_t> presentes;
    for (const EstadoPersonaje& jugador: estado) {
        presentes.insert(jugador.id);
        auto it = personajes.try_emplace(jugador.id, sprite_personaje, ESCALA).first;
        it->second.actualizar(SDL2pp::Point(jugador.x, jugador.y));
    }
    std::erase_if(personajes,
                  [&presentes](const auto& par) { return !presentes.contains(par.first); });
}

void Interfaz::dibujar() {
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    auto propio = personajes.find(ID_PROPIO);
    if (propio != personajes.end()) {
        // TODO: pasar el tamaño del mapa cuando llegue del servidor; con 0 la camara no se mueve
        SDL2pp::Rect camara = calculador_camara.calcular(propio->second.centro(), 0, 0);
        for (const auto& [id, personaje]: personajes) {
            personaje.dibujar(renderer, camara);
        }
    }
    renderer.Present();
}
