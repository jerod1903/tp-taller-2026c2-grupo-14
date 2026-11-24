#include "interfaz.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <SDL_image.h>

#include "client/audio/sonidos_granja.h"
#include "client/config/client_config.h"
#include "client/constants.h"
#include "client/gui/rutas.h"
#include "client/gui/sprites/constantes_herramienta_en_mano.h"
#include "client/gui/sprites/golpe_espada.h"
#include "client/gui/sprites/herramienta_en_mano.h"
#include "client/mapa-general/grilla_mapa.h"
#include "client/mapa-general/mundo.h"
#include "common/map/CabinInterior.h"

Interfaz::Interfaz():
        sdl(SDL_INIT_VIDEO),
        sdl_image(IMG_INIT_PNG),
        ventana("Jueguito", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                client_config().ventana.ancho, client_config().ventana.alto,
                SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        renderer(ventana, -1,
                 SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE),
        constructor_capas(renderer),
        hoja_vaca(renderer, RUTA_HOJA_VACA),
        hoja_gallina(renderer, RUTA_HOJA_GALLINA),
        hoja_cultivos(renderer, RUTA_HOJA_CULTIVOS),
        hoja_minerales(renderer, RUTA_HOJA_MINERALES),
        icono_regadera(renderer, RUTA_ICONO_REGADERA),
        aspectos{{EntityType::Player, {&FORMATO_PERSONAJE, {}}},
                 {EntityType::Cow, {&FORMATO_VACA, {capa_animal(hoja_vaca, FORMATO_VACA)}}},
                 {EntityType::Chicken,
                  {&FORMATO_GALLINA, {capa_animal(hoja_gallina, FORMATO_GALLINA)}}}},
        barra(renderer),
        fuente(RUTA_FUENTE, TAMANO_FUENTE_CHAT),
        fuente_titulo(RUTA_FUENTE, TAMANO_FUENTE_TITULO),
        panel_chat(fuente),
        menu_pausa(renderer, fuente, fuente_titulo),
        pantalla_fin_del_dia(renderer, fuente, fuente_titulo),
        hud(renderer),
        globo(renderer),
        transicion(DURACION_TRANSICION) {
    for (const auto& [item, ruta]: RUTAS_ICONOS) {
        objetos_piso.try_emplace(item, renderer, ruta);
    }
    for (const auto& [tipo, hojas]: HOJAS_POR_POSE) {
        auto tinte = TINTES.find(tipo);
        for (const auto& [pose, hoja]: hojas) {
            SDL2pp::Texture& textura =
                    hojas_por_pose.try_emplace(hoja.ruta, renderer, hoja.ruta).first->second;
            Capa capa = capa_animal(textura, hoja.formato);
            capa.desplazamiento = SDL2pp::Point(hoja.corrimiento_x, hoja.corrimiento_y);
            if (tinte != TINTES.end()) {
                capa.tinte = tinte->second;
            }
            aspectos_por_pose[tipo].emplace(pose, Aspecto{&hoja.formato, {capa}});
        }
    }
}

Capa Interfaz::capa_animal(SDL2pp::Texture& hoja, const FormatoAnimacion& formato) {
    SDL2pp::Point cuadro(formato.ancho_cuadro, formato.alto_cuadro);
    return Capa{&hoja, TipoCapa::PorCuadro, SDL2pp::Point(0, 0), cuadro, SDL2pp::Point(0, 0)};
}

namespace {
bool inconsciente(const EstadoJuego& estado, uint32_t player_id) {
    return std::any_of(estado.jugadores.begin(), estado.jugadores.end(),
                       [player_id](const PlayerState& jugador) {
                           return jugador.playerId == player_id && jugador.unconscious;
                       });
}

std::optional<ItemType> item_en_mano(const EstadoJuego& estado, uint32_t player_id) {
    const InventoryState& inventario = estado.inventario;
    if (player_id != estado.id_propio || inventario.selectedSlot >= inventario.slots.size()) {
        return std::nullopt;
    }
    return inventario.slots[inventario.selectedSlot].item;
}

void borrar_ultima_letra(std::string& texto) {
    while (!texto.empty() && (static_cast<unsigned char>(texto.back()) & 0xC0) == 0x80) {
        texto.pop_back();
    }
    if (!texto.empty()) {
        texto.pop_back();
    }
}
}  // namespace

Entrada Interfaz::leer_entrada() {
    Entrada entrada;
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        if (evento.type == SDL_QUIT) {
            entrada.salir = true;
        }
        if (menu_pausa.abierto()) {
            leer_pausa(evento, entrada);
            continue;
        }
        if (borrador) {
            leer_chat(evento, entrada);
            continue;
        }
        if (fin_del_dia_abierto) {
            leer_fin_del_dia(evento, entrada);
            continue;
        }
        if (mapa_general_abierto()) {
            leer_mapa_general(evento);
            continue;
        }
        if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT &&
            camara) {
            entrada.click = casilla_clickeada(evento.button.x, evento.button.y);
            continue;
        }
        if (evento.type != SDL_KEYDOWN || evento.key.repeat) {
            continue;
        }
        SDL_Keycode tecla = evento.key.keysym.sym;
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
    // Escribiendo, en pausa, con el día terminado o mirando el mapa no camina
    if (borrador || menu_pausa.abierto() || fin_del_dia_abierto || mapa_general_abierto()) {
        return entrada;
    }
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
    entrada.dir_x = static_cast<int8_t>(dir_x);
    entrada.dir_y = static_cast<int8_t>(dir_y);
    return entrada;
}

void Interfaz::leer_pausa(const SDL_Event& evento, Entrada& entrada) {
    std::optional<OpcionPausa> opcion = menu_pausa.leer(evento, renderer.GetOutputSize());
    if (opcion == OpcionPausa::Salir) {
        entrada.salir = true;
    } else if (opcion == OpcionPausa::Invitar) {
        entrada.invitar = true;
    }
}

void Interfaz::leer_fin_del_dia(const SDL_Event& evento, Entrada& entrada) {
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

void Interfaz::leer_mapa_general(const SDL_Event& evento) {
    mapa_general->leer(evento);
    if (evento.type == SDL_KEYDOWN && !evento.key.repeat && evento.key.keysym.sym == SDLK_F11) {
        alternar_pantalla_completa();
    }
}

bool Interfaz::mapa_general_abierto() const { return mapa_general && mapa_general->abierto(); }

ClickRequest Interfaz::casilla_clickeada(int x, int y) const {
    int x_mundo = x / escala + camara->x;
    int y_mundo = y / escala + camara->y;
    return ClickRequest{static_cast<uint16_t>(x_mundo / TAMANO_TILE),
                        static_cast<uint16_t>(y_mundo / TAMANO_TILE)};
}

void Interfaz::leer_chat(const SDL_Event& evento, Entrada& entrada) {
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

void Interfaz::alternar_pantalla_completa() {
    pantalla_completa = !pantalla_completa;
    ventana.SetFullscreen(pantalla_completa ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void Interfaz::actualizar_mapa(const EstadoJuego& estado) {
    if (estado.mundo && (!mapa || *estado.mundo != mundo_del_mapa)) {
        // Se lee una sola vez: lo usan el área que se ve y el mapa general
        const stardew::maps::MapDefinition mundo = cargar_mundo(*estado.mundo);
        mapa.emplace(renderer, mundo, estado.max_jugadores);
        GrillaMapa grilla = grilla_del_mapa(mundo);
        minimapa.emplace(renderer, grilla);
        mapa_general.emplace(renderer, std::move(grilla));
        mundo_del_mapa = *estado.mundo;
    }
}

void Interfaz::actualizar_entidades(const EstadoJuego& estado,
                                    std::chrono::steady_clock::time_point ahora) {
    std::set<std::pair<EntityType, uint32_t>> presentes;
    for (const EntityState& entidad: estado.entidades) {
        auto aspecto = aspectos.find(entidad.type);
        auto por_pose = aspectos_por_pose.find(entidad.type);
        if (aspecto == aspectos.end() && por_pose == aspectos_por_pose.end()) {
            continue;
        }
        std::pair<EntityType, uint32_t> clave(entidad.type, entidad.entityId);
        presentes.insert(clave);
        auto [it, nueva] = por_pose != aspectos_por_pose.end() ?
                                   entidades.try_emplace(clave, por_pose->second) :
                                   entidades.try_emplace(clave, *aspecto->second.formato,
                                                         aspecto->second.capas);
        if (entidad.type == EntityType::Player) {
            vestir(it->second, entidad.entityId, estado, nueva);
            it->second.tirar(inconsciente(estado, entidad.entityId));
            it->second.sostener(item_en_mano(estado, entidad.entityId));
        }
        it->second.actualizar(entidad, estado.tick, estado.duracion_tick, ahora);
    }
    for (auto it = entidades.begin(); it != entidades.end();) {
        if (presentes.contains(it->first)) {
            ++it;
            continue;
        }
        if (it->first.first == EntityType::Player) {
            vestidos.erase(it->first.second);
            constructor_capas.olvidar(it->first.second);
        }
        it = entidades.erase(it);
    }
}

void Interfaz::vestir(EntidadGrafica& jugador, uint32_t player_id, const EstadoJuego& estado,
                      bool recien_creado) {
    auto info = estado.info_jugadores.find(player_id);
    bool tiene_apariencia = info != estado.info_jugadores.end();
    if (!recien_creado && (!tiene_apariencia || vestidos.contains(player_id))) {
        return;
    }
    std::optional<CharacterAppearance> elegida;
    if (tiene_apariencia) {
        elegida = info->second.appearance;
        vestidos.insert(player_id);
    }
    jugador.vestir(constructor_capas.capas(player_id, elegida));
}

void Interfaz::sonar_granja(const FarmState& granja, const std::string& area_propia) {
    if (granja_anterior == granja) {
        return;
    }
    if (granja_anterior && granja.areaId == area_propia) {
        for (Sonido sonido: sonidos_granja(*granja_anterior, granja)) {
            reproductor.reproducir(sonido);
        }
    }
    granja_anterior = granja;
}

void Interfaz::actualizar_sonidos(const EstadoJuego& estado,
                                  std::chrono::steady_clock::time_point ahora) {
    for (const auto& [item, sonido]: SONIDOS_AL_RECIBIR) {
        int cantidad = 0;
        for (const InventorySlot& slot: estado.inventario.slots) {
            cantidad += slot.item == item ? slot.quantity : 0;
        }
        auto anterior = cantidades_anteriores.find(item);
        if (anterior != cantidades_anteriores.end() && cantidad > anterior->second) {
            reproductor.reproducir(sonido);
        }
        cantidades_anteriores[item] = cantidad;
    }
    auto propio = std::find_if(
            estado.jugadores.begin(), estado.jugadores.end(),
            [&estado](const PlayerState& jugador) { return jugador.playerId == estado.id_propio; });
    if (propio == estado.jugadores.end()) {
        return;
    }
    sonar_granja(estado.granja, propio->areaId);
    if (estado.tick != tick_escuchado) {
        if (propio_anterior) {
            bool tiene_direccion = propio->dirX != 0 || propio->dirY != 0;
            bool se_movio = propio->x != propio_anterior->x || propio->y != propio_anterior->y;
            caminando = tiene_direccion && se_movio;
            if (propio->gold > propio_anterior->gold) {
                reproductor.reproducir(Sonido::Moneda);
            }
        }
        propio_anterior = *propio;
        tick_escuchado = estado.tick;
    }
    if (caminando && ahora >= proximo_paso) {
        reproductor.reproducir(Sonido::Paso);
        proximo_paso = ahora + client_config().audio.intervalo_paso;
    }
}

void Interfaz::ajustar_lienzo(int ancho_area, int alto_area) {
    SDL2pp::Point salida = renderer.GetOutputSize();
    escala = std::max(1, salida.y / client_config().ventana.alto_mundo_visible);
    if (ancho_area > 0 && alto_area > 0) {
        escala = std::max({escala, (salida.x + ancho_area - 1) / ancho_area,
                           (salida.y + alto_area - 1) / alto_area});
    }
    int ancho = std::max(1, (salida.x + escala - 1) / escala);
    int alto = std::max(1, (salida.y + escala - 1) / escala);
    if (!lienzo || lienzo->GetWidth() != ancho || lienzo->GetHeight() != alto) {
        lienzo.emplace(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, ancho, alto);
        calculador_camara.ajustar_vista(ancho, alto);
    }
}

void Interfaz::dibujar(const EstadoJuego& estado) {
    auto ahora = std::chrono::steady_clock::now();
    actualizar_mapa(estado);
    actualizar_entidades(estado, ahora);
    camas.clear();
    std::copy_if(estado.entidades.begin(), estado.entidades.end(), std::back_inserter(camas),
                 [](const EntityState& entidad) { return entidad.type == EntityType::Bed; });
    actualizar_sonidos(estado, ahora);
    auto propio = entidades.find({EntityType::Player, estado.id_propio});
    if (propio != entidades.end() && propio->second.area() != area_anterior) {
        if (!area_anterior.empty()) {
            congelar_lienzo(ahora);
        }
        area_anterior = propio->second.area();
    }
    bool hay_area = mapa && propio != entidades.end();
    if (hay_area) {
        mapa->mostrar_area(propio->second.area());
    }
    ajustar_lienzo(hay_area ? mapa->ancho_px() : 0, hay_area ? mapa->alto_px() : 0);
    renderer.SetTarget(*lienzo);
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    if (propio != entidades.end()) {
        if (transicion.oscureciendo(ahora) && congelado) {
            renderer.Copy(*congelado);
        } else {
            dibujar_area(propio->second, estado.granja, estado.mina, ahora);
        }
        oscurecer(transicion.opacidad(ahora));
        dibujar_inventario(estado.inventario);
    }
    renderer.SetTarget();
    renderer.Clear();
    renderer.Copy(*lienzo, SDL2pp::NullOpt,
                  SDL2pp::Rect(0, 0, lienzo->GetWidth() * escala, lienzo->GetHeight() * escala));
    fin_del_dia_abierto = estado.fin_del_dia.has_value();
    // con el mapa general abierto el fin del día va encima del mapa
    if (estado.fin_del_dia && !mapa_general_abierto()) {
        pantalla_fin_del_dia.dibujar(renderer, renderer.GetOutputSize(), *estado.fin_del_dia,
                                     estado.listo_para_manana);
    }
    auto info_propia = estado.info_jugadores.find(estado.id_propio);
    std::string nombre_propio =
            info_propia != estado.info_jugadores.end() ? info_propia->second.characterName : "";
    panel_chat.dibujar(renderer, estado.chat, borrador, renderer.GetOutputSize(), nombre_propio,
                       ahora);
    hud.dibujar(renderer, estado, escala);
    if (minimapa) {
        minimapa->dibujar(renderer, estado.jugadores, estado.id_propio, escala_del_hud(escala));
    }
    if (mapa_general_abierto()) {
        mapa_general->dibujar(renderer, renderer.GetOutputSize(), estado.jugadores,
                              estado.id_propio);
        if (estado.fin_del_dia) {
            pantalla_fin_del_dia.dibujar(renderer, renderer.GetOutputSize(), *estado.fin_del_dia,
                                         estado.listo_para_manana);
        }
    }
    if (menu_pausa.abierto()) {
        menu_pausa.dibujar(renderer, renderer.GetOutputSize());
    }
    renderer.Present();
}

void Interfaz::dibujar_inventario(const InventoryState& inventario) {
    barra.dibujar(renderer, inventario, SDL2pp::Point(lienzo->GetWidth(), lienzo->GetHeight()));
}

void Interfaz::congelar_lienzo(std::chrono::steady_clock::time_point ahora) {
    if (!lienzo) {
        return;
    }
    congelado.emplace(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
                      lienzo->GetWidth(), lienzo->GetHeight());
    renderer.SetTarget(*congelado);
    renderer.Copy(*lienzo);
    renderer.SetTarget();
    transicion.empezar(ahora);
}

void Interfaz::oscurecer(uint8_t opacidad) {
    if (opacidad == 0) {
        return;
    }
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(0, 0, 0, opacidad);
    renderer.FillRect(SDL2pp::Rect(0, 0, lienzo->GetWidth(), lienzo->GetHeight()));
}

std::optional<Sprite> Interfaz::sprite_golpe_espada(const EntidadGrafica& entidad,
                                                    std::chrono::steady_clock::time_point ahora) {
    std::optional<float> avance = entidad.avance_ataque(ahora);
    if (!avance) {
        return std::nullopt;
    }
    GolpeEspada golpe = golpe_espada_de(entidad.mirando(), *avance);
    SDL2pp::Texture& espada = objetos_piso.at(ItemType::Sword);
    const auto& [agarre_x, agarre_y] = EMPUNADURAS.at(ItemType::Sword);
    SDL2pp::Point pies = entidad.pies();
    SDL2pp::Point tamano = espada.GetSize();
    SDL2pp::Point mano(pies.x + golpe.dx, pies.y + golpe.dy);
    Sprite sprite{&espada, SDL2pp::Rect(SDL2pp::Point(0, 0), tamano),
                  SDL2pp::Rect(mano.x - agarre_x, mano.y - agarre_y, tamano.x, tamano.y)};
    sprite.angulo = golpe.angulo;
    sprite.pivote = SDL2pp::Point(agarre_x, agarre_y);
    sprite.apoyo = golpe.detras ? pies.y - 1 : pies.y + 1;
    return sprite;
}

std::optional<Sprite> Interfaz::sprite_en_mano(const EntidadGrafica& entidad,
                                               std::chrono::steady_clock::time_point ahora) {
    if (std::optional<Sprite> golpe = sprite_golpe_espada(entidad, ahora)) {
        return golpe;
    }
    std::optional<ItemType> item = entidad.en_mano();
    if (!item) {
        return std::nullopt;
    }
    auto textura = objetos_piso.find(*item);
    if (textura == objetos_piso.end()) {
        return std::nullopt;
    }
    SDL2pp::Point tamano = textura->second.GetSize();
    std::optional<HerramientaEnMano> herramienta =
            herramienta_en_mano_de(*item, entidad.mirando(), tamano.x);
    if (!herramienta) {
        return std::nullopt;
    }
    SDL2pp::Point pies = entidad.pies();
    Sprite sprite{&textura->second, SDL2pp::Rect(SDL2pp::Point(0, 0), tamano),
                  SDL2pp::Rect(pies.x + herramienta->dx, pies.y + herramienta->dy, tamano.x,
                               tamano.y),
                  herramienta->espejada};
    sprite.apoyo = herramienta->detras ? pies.y - 1 : pies.y + 1;
    return sprite;
}

std::optional<Sprite> Interfaz::sprite_objeto(const GroundItem& objeto) {
    auto mineral = ICONOS_MINERALES.find(objeto.item);
    if (mineral != ICONOS_MINERALES.end()) {
        const SDL2pp::Rect& origen = mineral->second;
        SDL2pp::Point esquina(objeto.x * TAMANO_TILE + (TAMANO_TILE - origen.w) / 2,
                              objeto.y * TAMANO_TILE + (TAMANO_TILE - origen.h) / 2);
        Sprite sprite{&hoja_minerales, origen, SDL2pp::Rect(esquina, origen.GetSize())};
        sprite.apoyo = objeto.y * TAMANO_TILE;
        return sprite;
    }
    auto textura = objetos_piso.find(objeto.item);
    if (textura == objetos_piso.end()) {
        return std::nullopt;
    }
    SDL2pp::Point tamano = textura->second.GetSize();
    SDL2pp::Point esquina(objeto.x * TAMANO_TILE + (TAMANO_TILE - tamano.x) / 2,
                          objeto.y * TAMANO_TILE + (TAMANO_TILE - tamano.y) / 2);
    return Sprite{&textura->second, SDL2pp::Rect(SDL2pp::Point(0, 0), tamano),
                  SDL2pp::Rect(esquina, tamano)};
}

Sprite Interfaz::sprite_veta(const OreDeposit& veta) {
    const SDL2pp::Rect& origen = VETAS_MINERALES.at(veta.ore);
    SDL2pp::Rect destino(veta.x * TAMANO_TILE + (TAMANO_TILE - origen.w) / 2,
                         (veta.y + 1) * TAMANO_TILE - origen.h, origen.w, origen.h);
    Sprite sprite{&hoja_minerales, origen, destino};
    if (veta.ore == ItemType::Silver) {
        sprite.tinte = {210, 235, 255, 255};
    }
    return sprite;
}

Sprite Interfaz::sprite_cultivo(const FarmTile& celda) {
    SDL2pp::Rect origen(celda.stage * TAMANO_CUADRO_CULTIVO,
                        FILAS_CULTIVOS.at(celda.crop) * TAMANO_CUADRO_CULTIVO,
                        TAMANO_CUADRO_CULTIVO, TAMANO_CUADRO_CULTIVO);
    SDL2pp::Rect destino(celda.x * TAMANO_TILE, celda.y * TAMANO_TILE, TAMANO_TILE, TAMANO_TILE);
    return Sprite{&hoja_cultivos, origen, destino};
}

void Interfaz::dibujar_aviso(const FarmTile& celda, const SDL2pp::Rect& camara) {
    SDL2pp::Point arriba(celda.x * TAMANO_TILE + TAMANO_TILE / 2, celda.y * TAMANO_TILE);
    if (celda.readyToHarvest) {
        SDL2pp::Rect producto(COLUMNAS_CALIDAD.at(ItemQuality::Base) * TAMANO_CUADRO_CULTIVO,
                              FILAS_CULTIVOS.at(celda.crop) * TAMANO_CUADRO_CULTIVO,
                              TAMANO_CUADRO_CULTIVO, TAMANO_CUADRO_CULTIVO);
        globo.dibujar_icono(renderer, hoja_cultivos, producto, arriba, camara);
    } else if (celda.needsWater) {
        globo.dibujar_icono(renderer, icono_regadera,
                            SDL2pp::Rect(SDL2pp::Point(0, 0), icono_regadera.GetSize()), arriba,
                            camara);
    }
}

void Interfaz::dibujar_tierra(const FarmState& granja, const SDL2pp::Rect& camara) {
    for (const FarmTile& celda: granja.tiles) {
        mapa->dibujar_tile(renderer, TILES_SUELO.at(celda.soil), celda.x, celda.y, camara);
    }
}

void Interfaz::dibujar_area(const EntidadGrafica& propio, const FarmState& granja,
                            const std::map<std::string, DungeonState>& mina,
                            std::chrono::steady_clock::time_point ahora) {
    const std::string& area = propio.area();
    int ancho_mapa = mapa ? mapa->ancho_px() : 0;
    int alto_mapa = mapa ? mapa->alto_px() : 0;
    SDL2pp::Rect camara = calculador_camara.calcular(propio.centro(), ancho_mapa, alto_mapa);
    this->camara = camara;
    std::vector<Sprite> sprites;
    if (mapa) {
        mapa->dibujar_suelo(renderer, camara);
        if (granja.areaId == area) {
            dibujar_tierra(granja, camara);
        }
        sprites = mapa->elementos();
    }
    for (const EntityState& cama: camas) {
        if (!mapa || cama.areaId != area) {
            continue;
        }
        namespace maps = stardew::maps;
        int x = static_cast<int>(cama.x / POSITION_SCALE);
        int y = static_cast<int>(cama.y / POSITION_SCALE);
        int alto_frazada = maps::BED_HEIGHT - maps::BED_HEAD_HEIGHT;
        for (std::optional<Sprite> parte:
             {mapa->sprite_elemento(maps::ElementType::Bed, x, y, maps::BED_WIDTH,
                                    maps::BED_HEAD_HEIGHT),
              mapa->sprite_elemento(maps::ElementType::BedBlanket, x, y + maps::BED_HEAD_HEIGHT,
                                    maps::BED_WIDTH, alto_frazada)}) {
            if (parte) {
                sprites.push_back(*parte);
            }
        }
    }
    if (granja.areaId == area) {
        for (const FarmTile& celda: granja.tiles) {
            if (celda.crop != CropType::None) {
                sprites.push_back(sprite_cultivo(celda));
            }
        }
        for (const GroundItem& objeto: granja.items) {
            if (std::optional<Sprite> sprite = sprite_objeto(objeto)) {
                sprites.push_back(*sprite);
            }
        }
    }
    auto mina_actual = mina.find(area);
    if (mina_actual != mina.end()) {
        for (const OreDeposit& veta: mina_actual->second.ores) {
            sprites.push_back(sprite_veta(veta));
        }
        for (const GroundItem& objeto: mina_actual->second.items) {
            if (std::optional<Sprite> sprite = sprite_objeto(objeto)) {
                sprites.push_back(*sprite);
            }
        }
    }
    for (const auto& [clave, entidad]: entidades) {
        if (entidad.area() == area) {
            std::vector<Sprite> capas = entidad.sprites();
            if (EN_EL_PISO.contains(clave.first)) {
                // Debajo de cualquiera que esté parado en su celda
                for (Sprite& capa: capas) {
                    capa.apoyo = capa.destino.y;
                }
            }
            sprites.insert(sprites.end(), capas.begin(), capas.end());
            if (clave.first == EntityType::Player) {
                if (std::optional<Sprite> mano = sprite_en_mano(entidad, ahora)) {
                    sprites.push_back(*mano);
                }
            }
        }
    }
    std::stable_sort(sprites.begin(), sprites.end(),
                     [](const Sprite& a, const Sprite& b) { return a.base() < b.base(); });
    for (const Sprite& sprite: sprites) {
        sprite.dibujar(renderer, camara);
    }
    for (const auto& [clave, entidad]: entidades) {
        if (entidad.area() == area && entidad.emote() != Emote::None) {
            globo.dibujar(renderer, entidad.emote(), entidad.cabeza(), camara);
        }
    }
    if (granja.areaId == area) {
        for (const FarmTile& celda: granja.tiles) {
            dibujar_aviso(celda, camara);
        }
    }
}
