#include "client/config/client_config.h"

#include <stdexcept>
#include <string>

#include <toml++/toml.hpp>

#include "common/map/persistence/TomlTableReader.h"

using stardew::maps::detail::TomlTableReader;

namespace {

const char* const ARCHIVO = "client_config.toml";

TomlTableReader seccion(const toml::table& raiz, const std::string& nombre) {
    const toml::table* tabla = raiz[nombre].as_table();
    if (tabla == nullptr) {
        throw std::runtime_error(std::string(ARCHIVO) + " is missing section [" + nombre + "]");
    }
    return TomlTableReader(*tabla, "[" + nombre + "]");
}

int positivo(const TomlTableReader& lector, std::string_view clave) {
    int valor = lector.requiredInt(clave);
    if (valor <= 0) {
        throw std::runtime_error(std::string(ARCHIVO) + ": '" + std::string(clave) +
                                 "' must be positive");
    }
    return valor;
}

}  // namespace

ClientConfig cargar_client_config(const std::string& ruta) {
    toml::table raiz;
    try {
        raiz = toml::parse_file(ruta);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error("could not read " + ruta + ": " + std::string(e.description()));
    }
    TomlTableReader ventana = seccion(raiz, "window");
    TomlTableReader audio = seccion(raiz, "audio");
    int fps = positivo(ventana, "fps");
    return ClientConfig{
            {positivo(ventana, "width"), positivo(ventana, "height"),
             positivo(ventana, "visible_world_height"), std::chrono::milliseconds(1000 / fps)},
            {positivo(audio, "channels"), positivo(audio, "buffer_size"),
             std::chrono::milliseconds(positivo(audio, "step_interval_ms")),
             positivo(audio, "music_volume")}};
}

const ClientConfig& client_config() {
    static const ClientConfig config =
            cargar_client_config(std::string(TALLER_CONFIG_DIR) + "/" + ARCHIVO);
    return config;
}
