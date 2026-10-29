#pragma once

#include "client/menu/screens/BaseScreen.h"
#include "common/lobby_dto.h"

class CharacterPreview;
class ColorPicker;
class OptionStepper;
class QLabel;
class QLineEdit;
class MenuButton;
class QShowEvent;

// Nombre y apariencia del personaje. La usan los dos flujos (crear partida y unirse):
// la pantalla no habla con el servidor, solo emite lo elegido. MainWindow arma el pedido
// y, si el servidor lo rechaza, le pasa el error con showError().
class CharacterScreen: public BaseScreen {
    Q_OBJECT

public:
    explicit CharacterScreen(QWidget* parent = nullptr);

    void showError(LobbyStatus status);

signals:
    void characterChosen(const QString& name, const CharacterAppearance& appearance);

protected:
    // Cada vez que se entra a la pantalla arranca con un personaje nuevo.
    void showEvent(QShowEvent* event) override;

private:
    QWidget* createForm();
    CharacterAppearance currentAppearance() const;
    void refreshPreview();
    void confirm();

    CharacterPreview* preview;
    QLineEdit* nameInput;
    OptionStepper* hairStyle;
    ColorPicker* hairColor;
    OptionStepper* shirt;
    OptionStepper* pants;
    OptionStepper* hat;
    QLabel* errorLabel;
    MenuButton* confirmButton;
};
