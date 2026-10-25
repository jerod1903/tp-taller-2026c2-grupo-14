#include "interfaz_juego.h"
#include "constants.h"
#include <SDL2/SDL_image.h>
#include <map>
#include <utility>

InterfazJuego::InterfazJuego():
        sdl(SDL_INIT_VIDEO),
        sdl_image(IMG_INIT_PNG),
        ventana(
            "Jueguito", 
            SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, 
            ANCHO_VENTANA,
            ALTO_VENTANA, 
            SDL_WINDOW_SHOWN
        ),
        renderer(
            ventana,
            -1, 
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        ),
        caverna(renderer, ESCALA),
        personaje(renderer, ESCALA, caverna.posicion_inicial()),
        calculador_camara(ANCHO_VENTANA, ALTO_VENTANA) {}

void InterfazJuego::ejecutar() {
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

void InterfazJuego::procesar_eventos() {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        if (evento.type == SDL_QUIT ||
            (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_ESCAPE)) {
            corriendo = false;
        }
    }
}

void InterfazJuego::actualizar() {
    static std::map<std::pair<int, int>, std::pair<SDL_Scancode, SDL_Scancode>> teclas_direccion = {
        {{-1, 0}, {SDL_SCANCODE_LEFT, SDL_SCANCODE_A}},
        {{1, 0}, {SDL_SCANCODE_RIGHT, SDL_SCANCODE_D}},
        {{0, -1}, {SDL_SCANCODE_UP, SDL_SCANCODE_W}},
        {{0, 1}, {SDL_SCANCODE_DOWN, SDL_SCANCODE_S}}
    };
    const Uint8* teclas = SDL_GetKeyboardState(nullptr);
    int dir_x = 0;
    int dir_y = 0;
    for (const auto& [direccion, par_teclas] : teclas_direccion) {
        if (teclas[par_teclas.first] || teclas[par_teclas.second]) {
            dir_x += direccion.first;
            dir_y += direccion.second;
        }
    }
    if (dir_x == 0 && dir_y == 0) {
        return;
    }
    Request request{};
    request.tipo = Operation::Move;
    request.dir_x = (dir_x > 0) ? 1 : (dir_x < 0) ? 2 : 0;
    request.dir_y = (dir_y < 0) ? 1 : (dir_y > 0) ? 2 : 0;
    enviar_request(request);
    personaje.mover(dir_x * VELOCIDAD_MOV, dir_y * VELOCIDAD_MOV, caverna);
}

// TODO: pasarle la request al protocolo cuando este implementado
void InterfazJuego::enviar_request(const Request& /*request*/) {}

void InterfazJuego::dibujar() {
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    SDL2pp::Rect camara = calculador_camara.calcular(personaje.centro(), caverna.ancho_px(),
                                                     caverna.alto_px());
    caverna.dibujar(renderer, camara);
    personaje.dibujar(renderer, camara);
    renderer.Present();
}
