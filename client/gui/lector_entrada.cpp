#include "lector_entrada.h"

#include <cstring>
#include <map>
#include <string>
#include <utility>

#include "client/constants.h"

namespace {
void borrar_ultima_letra(std::string& texto) {
    while (!texto.empty() && (static_cast<unsigned char>(texto.back()) & 0xC0) == 0x80) {
        texto.pop_back();
    }
    if (!texto.empty()) {
        texto.pop_back();
    }
}
}  // namespace

LectorEntrada::LectorEntrada(SDL2pp::Window& ventana, SDL2pp::Renderer& renderer,
                             ControladorSonidos& controlador_sonidos, MenuPausa& menu_pausa,
                             PantallaFinDelDia& pantalla_fin_del_dia, PanelCofre& panel_cofre,
                             std::optional<MapaGeneral>& mapa_general):
        ventana(ventana),
        renderer(renderer),
        controlador_sonidos(controlador_sonidos),
        menu_pausa(menu_pausa),
        pantalla_fin_del_dia(pantalla_fin_del_dia),
        panel_cofre(panel_cofre),
        mapa_general(mapa_general) {}

Entrada LectorEntrada::leer(const Vista& vista_actual) {
    vista = vista_actual;
    Entrada entrada;
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        if (evento.type == SDL_QUIT) {
            entrada.salir = true;
        }
        leer_evento(evento, entrada);
    }
    if (!hay_modo_abierto()) {
        leer_direccion(entrada);
    }
    return entrada;
}

const std::optional<std::string>& LectorEntrada::borrador_chat() const {
    return borrador;
}

void LectorEntrada::leer_evento(const SDL_Event& evento, Entrada& entrada) {
    if (menu_pausa.abierto()) {
        leer_pausa(evento, entrada);
    } else if (borrador) {
        leer_chat(evento, entrada);
    } else if (vista.fin_del_dia_abierto) {
        leer_fin_del_dia(evento, entrada);
    } else if (mapa_general_abierto()) {
        leer_mapa_general(evento);
    } else if (vista.cofre_abierto) {
        leer_cofre(evento, entrada);
    } else if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT &&
               vista.camara) {
        entrada.click = casilla_clickeada(evento.button.x, evento.button.y);
    } else if (evento.type == SDL_KEYDOWN && !evento.key.repeat) {
        leer_tecla(evento.key.keysym.sym, entrada);
    }
}

void LectorEntrada::leer_tecla(SDL_Keycode tecla, Entrada& entrada) {
    if (tecla == SDLK_ESCAPE) {
        menu_pausa.abrir();
    } else if (tecla == SDLK_RETURN) {
        borrador = "";
        entrada.escribiendo = true;
    } else if (tecla == SDLK_e) {
        entrada.interactuar = true;
    } else if (tecla == SDLK_x) {
        entrada.cruzar_puerta = true;
    } else if (tecla == SDLK_F11) {
        alternar_pantalla_completa();
    } else if (tecla == SDLK_m && mapa_general) {
        mapa_general->abrir();
    } else if (tecla >= SDLK_1 && tecla <= SDLK_9) {
        entrada.slot_seleccionado = static_cast<uint8_t>(tecla - SDLK_1);
    }
}

void LectorEntrada::leer_direccion(Entrada& entrada) const {
    static const std::map<std::pair<int, int>, std::pair<SDL_Scancode, SDL_Scancode>>
            teclas_direccion = {{{-1, 0}, {SDL_SCANCODE_LEFT, SDL_SCANCODE_A}},
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
    entrada.dir_x = static_cast<int8_t>(dir_x);
    entrada.dir_y = static_cast<int8_t>(dir_y);
}

bool LectorEntrada::hay_modo_abierto() const {
    return borrador || menu_pausa.abierto() || vista.fin_del_dia_abierto ||
           mapa_general_abierto() || vista.cofre_abierto;
}

void LectorEntrada::leer_pausa(const SDL_Event& evento, Entrada& entrada) {
    std::optional<OpcionPausa> opcion = menu_pausa.leer(evento, renderer.GetOutputSize());
    if (opcion == OpcionPausa::Salir) {
        entrada.salir = true;
    } else if (opcion == OpcionPausa::Invitar) {
        entrada.invitar = true;
    } else if (opcion == OpcionPausa::Musica) {
        controlador_sonidos.alternar_musica();
    }
}

void LectorEntrada::leer_fin_del_dia(const SDL_Event& evento, Entrada& entrada) {
    if (pantalla_fin_del_dia.leer(evento, renderer.GetOutputSize())) {
        entrada.listo_para_manana = true;
        return;
    }
    if (evento.type != SDL_KEYDOWN || evento.key.repeat) {
        return;
    }
    SDL_Keycode tecla = evento.key.keysym.sym;
    if (tecla == SDLK_ESCAPE) {
        menu_pausa.abrir();
    } else if (tecla == SDLK_RETURN) {
        borrador = "";
        entrada.escribiendo = true;
    } else if (tecla == SDLK_F11) {
        alternar_pantalla_completa();
    }
}

void LectorEntrada::leer_mapa_general(const SDL_Event& evento) {
    mapa_general->leer(evento);
    if (evento.type == SDL_KEYDOWN && !evento.key.repeat && evento.key.keysym.sym == SDLK_F11) {
        alternar_pantalla_completa();
    }
}

void LectorEntrada::leer_cofre(const SDL_Event& evento, Entrada& entrada) {
    if (evento.type == SDL_MOUSEBUTTONDOWN) {
        std::optional<ChestSlotRequest> casillero =
                panel_cofre.casillero_en(SDL2pp::Point(evento.button.x / vista.escala_interfaz,
                                                       evento.button.y / vista.escala_interfaz));
        if (casillero && evento.button.button == SDL_BUTTON_LEFT) {
            entrada.transferencia = casillero;
        } else if (casillero && evento.button.button == SDL_BUTTON_RIGHT) {
            entrada.particion = casillero;
        }
        return;
    }
    if (evento.type != SDL_KEYDOWN || evento.key.repeat) {
        return;
    }
    SDL_Keycode tecla = evento.key.keysym.sym;
    if (tecla == SDLK_ESCAPE || tecla == SDLK_e) {
        entrada.cerrar_cofre = true;
        vista.cofre_abierto = false;
    } else if (tecla == SDLK_F11) {
        alternar_pantalla_completa();
    }
}

bool LectorEntrada::mapa_general_abierto() const {
    return mapa_general && mapa_general->abierto();
}

ClickRequest LectorEntrada::casilla_clickeada(int x, int y) const {
    int x_mundo = x / vista.escala + vista.camara->x;
    int y_mundo = y / vista.escala + vista.camara->y;
    return ClickRequest{static_cast<uint16_t>(x_mundo / TAMANO_TILE),
                        static_cast<uint16_t>(y_mundo / TAMANO_TILE)};
}

void LectorEntrada::leer_chat(const SDL_Event& evento, Entrada& entrada) {
    if (evento.type == SDL_TEXTINPUT &&
        borrador->size() + std::strlen(evento.text.text) <= MAX_CHAT_LENGTH) {
        borrador->append(evento.text.text);
    }
    if (evento.type != SDL_KEYDOWN) {
        return;
    }
    SDL_Keycode tecla = evento.key.keysym.sym;
    if (tecla == SDLK_BACKSPACE) {
        borrar_ultima_letra(*borrador);
    } else if (tecla == SDLK_RETURN) {
        if (!borrador->empty()) {
            entrada.mensaje_chat = *borrador;
        }
        borrador.reset();
        entrada.escribiendo = false;
    } else if (tecla == SDLK_ESCAPE) {
        borrador.reset();
        entrada.escribiendo = false;
    }
}

void LectorEntrada::alternar_pantalla_completa() {
    pantalla_completa = !pantalla_completa;
    ventana.SetFullscreen(pantalla_completa ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}
