#include "interfaz.h"

#include <map>
#include <optional>
#include <set>
#include <utility>
#include <variant>
#include <vector>

#include <SDL2/SDL_image.h>

#include "constants.h"

Interfaz::Interfaz(Enviador& enviador, Recibidor& recibidor, uint32_t id_propio):
        enviador(enviador),
        recibidor(recibidor),
        id_propio(id_propio),
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
            // Se avisa al servidor que el jugador sale de la partida
            enviador.enviar(Request{Operation::LeaveGame, 0, 0});
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
    // El servidor sigue moviendo al jugador en la ultima direccion que recibio: solo se avisa
    // cuando cambia, incluido el (0, 0) al soltar las teclas para que frene
    SDL2pp::Point direccion(dir_x, dir_y);
    if (direccion != ultima_direccion) {
        Request request{};
        request.tipo = Operation::Move;
        request.dir_x = static_cast<int8_t>(dir_x);
        request.dir_y = static_cast<int8_t>(dir_y);
        enviador.enviar(request);
        ultima_direccion = direccion;
    }
    // Se procesa todo lo que llego desde el frame anterior
    std::optional<ServerGameMessage> mensaje;
    while ((mensaje = recibidor.recibir())) {
        procesar_mensaje(*mensaje);
    }
    // El hilo termina si se corto la conexion
    if (!recibidor.is_alive()) {
        corriendo = false;
    }
    // Se dibuja el area en la que esta este jugador
    auto propio = personajes.find(id_propio);
    if (mapa && propio != personajes.end()) {
        mapa->mostrar_area(propio->second.area());
    }
}

void Interfaz::procesar_mensaje(const ServerGameMessage& mensaje) {
    if (const auto* ubicacion = std::get_if<LocationData>(&mensaje)) {
        mapa.emplace(renderer, ubicacion->worldId, ESCALA);
    } else if (const auto* snapshot = std::get_if<Snapshot>(&mensaje)) {
        actualizar_personajes(snapshot->players);
    } else {
        corriendo = false;  // GameEnded: termino la partida
    }
}

void Interfaz::actualizar_personajes(const std::vector<PlayerState>& jugadores) {
    std::set<uint32_t> presentes;
    for (const PlayerState& jugador: jugadores) {
        presentes.insert(jugador.playerId);
        auto it = personajes.try_emplace(jugador.playerId, sprite_personaje, ESCALA).first;
        it->second.actualizar(jugador);
    }
    std::erase_if(personajes,
                  [&presentes](const auto& par) { return !presentes.contains(par.first); });
}

void Interfaz::dibujar() {
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    // Sin personaje propio no se sabe que area mostrar
    auto propio = personajes.find(id_propio);
    if (propio != personajes.end()) {
        int ancho_mapa = mapa ? mapa->ancho_px() : 0;
        int alto_mapa = mapa ? mapa->alto_px() : 0;
        SDL2pp::Rect camara =
                calculador_camara.calcular(propio->second.centro(), ancho_mapa, alto_mapa);
        if (mapa) {
            mapa->dibujar(renderer, camara);
        }
        // Solo los que estan en la misma area que este jugador
        for (const auto& [id, personaje]: personajes) {
            if (personaje.area() == propio->second.area()) {
                personaje.dibujar(renderer, camara);
            }
        }
    }
    renderer.Present();
}
