#include "interfaz_juego.h"
#include "constants.h"
#include <SDL2/SDL_image.h>

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
    const Uint8* teclas = SDL_GetKeyboardState(nullptr);
    int dir_x = 0;
    int dir_y = 0;
    if (teclas[SDL_SCANCODE_LEFT] || teclas[SDL_SCANCODE_A]) {
        dir_x--;
    }
    if (teclas[SDL_SCANCODE_RIGHT] || teclas[SDL_SCANCODE_D]) {
        dir_x++;
    }
    if (teclas[SDL_SCANCODE_UP] || teclas[SDL_SCANCODE_W]) {
        dir_y--;
    }
    if (teclas[SDL_SCANCODE_DOWN] || teclas[SDL_SCANCODE_S]) {
        dir_y++;
    }
    if (dir_x == 0 && dir_y == 0) {
        return;
    }
    enviar_accion(armador_acciones.mover(dir_x, dir_y));
    personaje.mover(dir_x * VELOCIDAD_MOV, dir_y * VELOCIDAD_MOV, caverna);
}

void InterfazJuego::enviar_accion(const std::vector<uint8_t>& /*bytes*/) {}

void InterfazJuego::dibujar() {
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    SDL2pp::Rect camara = calculador_camara.calcular(personaje.centro(), caverna.ancho_px(),
                                                     caverna.alto_px());
    caverna.dibujar(renderer, camara);
    personaje.dibujar(renderer, camara);
    renderer.Present();
}
