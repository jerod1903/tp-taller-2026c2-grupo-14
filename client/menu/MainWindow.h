#pragma once

#include <QMainWindow>
#include <cstdint>

#include "client/menu/navigation/ScreenManager.h"
#include "common/lobby_dto.h"

class CharacterScreen;
class ILobbyProtocol;
class QStackedWidget;

// Ventana del menu previo a la partida. Cada pantalla es una pagina del QStackedWidget.
class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    // lobby: acceso al servidor; MainWindow se lo pasa a las pantallas que lo necesitan.
    explicit MainWindow(ILobbyProtocol& lobby, QWidget* parent = nullptr);

private:
    // Crea las pantallas y conecta sus senales con la navegacion.
    void setupScreens();
    // Con el personaje elegido manda al servidor el pedido del flujo en curso.
    void submitCharacter(const QString& name, const CharacterAppearance& appearance);

    // A la pantalla de personaje se llega desde dos flujos; esto dice cual y que pedir.
    enum class CharacterFlow { Create, Join };

    ILobbyProtocol& lobby;
    QStackedWidget* stack;
    ScreenManager screenManager;
    CharacterScreen* character = nullptr;

    CharacterFlow flow = CharacterFlow::Create;
    // Datos de los pasos anteriores, que se completan con el personaje.
    CreateGameRequest pendingCreate{};
    uint32_t pendingJoinGameId = 0;
};
