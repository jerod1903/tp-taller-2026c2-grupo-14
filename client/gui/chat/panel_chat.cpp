#include "client/gui/chat/panel_chat.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "client/constants.h"

namespace {
std::string texto_de(const ChatMessage& mensaje) {
    if (mensaje.kind == ChatKind::System) {
        return mensaje.text;
    }
    std::string privado = mensaje.kind == ChatKind::Private ? MARCA_PRIVADO : "";
    return mensaje.from + privado + ": " + mensaje.text;
}
}  // namespace

PanelChat::PanelChat(Fuente& fuente): fuente(fuente) {}

void PanelChat::dibujar(SDL2pp::Renderer& renderer, const std::vector<MensajeChat>& mensajes,
                        const std::optional<std::string>& borrador,
                        const SDL2pp::Point& tamano_ventana,
                        std::chrono::steady_clock::time_point ahora) {
    int ancho_maximo = tamano_ventana.x * PORCENTAJE_ANCHO_CHAT / 100;
    std::vector<Linea> lineas =
            lineas_visibles(mensajes, borrador.has_value(), ancho_maximo, ahora);
    if (borrador) {
        lineas.emplace_back(PROMPT_CHAT + *borrador + CURSOR_CHAT, COLOR_BORRADOR_CHAT);
    }
    if (lineas.empty()) {
        return;
    }
    int alto = static_cast<int>(lineas.size()) * fuente.alto_linea();
    int ancho = 0;
    for (const Linea& linea: lineas) {
        ancho = std::max(ancho, fuente.ancho(linea.first));
    }
    SDL2pp::Point posicion(MARGEN_CHAT, tamano_ventana.y - DISTANCIA_CHAT_ABAJO - alto);
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(COLOR_FONDO_CHAT.r, COLOR_FONDO_CHAT.g, COLOR_FONDO_CHAT.b,
                          COLOR_FONDO_CHAT.a);
    renderer.FillRect(SDL2pp::Rect(posicion.x - RELLENO_CHAT, posicion.y - RELLENO_CHAT,
                                   ancho + 2 * RELLENO_CHAT, alto + 2 * RELLENO_CHAT));
    for (const Linea& linea: lineas) {
        dibujar_linea(renderer, linea, posicion);
        posicion.y += fuente.alto_linea();
    }
}

std::vector<PanelChat::Linea> PanelChat::lineas_visibles(
        const std::vector<MensajeChat>& mensajes, bool abierto, int ancho_maximo,
        std::chrono::steady_clock::time_point ahora) const {
    std::vector<Linea> lineas;
    for (const MensajeChat& recibido: mensajes) {
        if (!abierto && ahora - recibido.llegada > DURACION_MENSAJE_CHAT) {
            continue;
        }
        for (const std::string& texto: partir(texto_de(recibido.mensaje), ancho_maximo)) {
            lineas.emplace_back(texto, COLORES_CHAT.at(recibido.mensaje.kind));
        }
    }
    if (lineas.size() > MAX_LINEAS_CHAT) {
        lineas.erase(lineas.begin(), lineas.end() - MAX_LINEAS_CHAT);
    }
    return lineas;
}

std::vector<std::string> PanelChat::partir(const std::string& texto, int ancho_maximo) const {
    std::vector<std::string> lineas;
    std::string actual;
    std::istringstream palabras(texto);
    std::string palabra;
    while (palabras >> palabra) {
        std::string probada = actual.empty() ? palabra : actual + " " + palabra;
        if (!actual.empty() && fuente.ancho(probada) > ancho_maximo) {
            lineas.push_back(actual);
            actual = palabra;
        } else {
            actual = probada;
        }
    }
    if (!actual.empty()) {
        lineas.push_back(actual);
    }
    return lineas;
}

void PanelChat::dibujar_linea(SDL2pp::Renderer& renderer, const Linea& linea,
                              const SDL2pp::Point& posicion) {
    SDL2pp::Texture textura = fuente.renderizar(renderer, linea.first, linea.second);
    renderer.Copy(textura, SDL2pp::NullOpt, SDL2pp::Rect(posicion, textura.GetSize()));
}
