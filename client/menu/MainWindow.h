#pragma once

#include <QMainWindow>
#include <QPixmap>
#include <string>

#include "client/app/GameSession.h"
#include "client/menu/navigation/ScreenManager.h"
#include "common/lobby_dto.h"

class CharacterScreen;
class ILobbyProtocol;
class QResizeEvent;
class QStackedWidget;

// Ventana del menu previo a la partida. Cada pantalla es una pagina del QStackedWidget.
class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    // lobby: acceso al servidor; MainWindow se lo pasa a las pantallas que lo necesitan.
    explicit MainWindow(ILobbyProtocol& lobby, QWidget* parent = nullptr);

signals:
    // El servidor dejo entrar a la partida: el menu termina y empieza el juego.
    void gameEntered(const GameSession& session);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    // Crea las pantallas y conecta sus senales con la navegacion.
    void setupScreens();
    // Con el personaje elegido manda al servidor el pedido del flujo en curso.
    void submitCharacter(const QString& name, const CharacterAppearance& appearance);
    // Deja el menu principal listo para cuando termine la partida y avisa que se entro.
    void enterGame(const GameEnteredResponse& response);
    // Ajusta el fondo elegido al tamano actual de la ventana.
    void updateBackground();

    // A la pantalla de personaje se llega desde dos flujos; esto dice cual y que pedir.
    enum class CharacterFlow { Create, Join };

    ILobbyProtocol& lobby;
    QStackedWidget* stack;
    ScreenManager screenManager;
    CharacterScreen* character = nullptr;
    // Fondo elegido al azar al abrir el menu; las pantallas son transparentes y lo dejan ver.
    QPixmap background;

    CharacterFlow flow = CharacterFlow::Create;
    // Datos de los pasos anteriores, que se completan con el personaje.
    CreateGameRequest pendingCreate{};
    std::string pendingInviteCode;
};
