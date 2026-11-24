#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include <chrono>
#include <string>

// Lo que se ajusta del cliente sin recompilar. Se lee de client_config.toml, en el root del repo.
struct ClientConfig {
    struct Ventana {
        int ancho;
        int alto;
        // En pixeles del tileset: la escala se elige para que entre mas o menos esto de alto
        int alto_mundo_visible;
        std::chrono::milliseconds duracion_frame;
    };
    struct Audio {
        int canales;  // cuantos efectos pueden sonar a la vez
        int tamano_buffer;
        std::chrono::milliseconds intervalo_paso;
        int volumen_musica;
    };

    Ventana ventana;
    Audio audio;
};

// Lanza std::runtime_error si el archivo no se puede leer, le falta un valor o uno es invalido
ClientConfig cargar_client_config(const std::string& ruta);

// La de client_config.toml, que se lee una sola vez
const ClientConfig& client_config();

#endif
