#include "client/menu/screens/CharacterScreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QShowEvent>
#include <QVBoxLayout>

#include "client/menu/components/CharacterPreview.h"
#include "client/menu/components/ColorPicker.h"
#include "client/menu/components/MenuButton.h"
#include "client/menu/components/OptionStepper.h"
#include "client/menu/styles/AppearanceCatalog.h"
#include "client/menu/styles/StyleManager.h"

namespace {
constexpr int PREVIEW_SIZE = 160;
// El mismo limite que aplica el servidor a los nombres.
constexpr int MAX_NAME_LENGTH = 20;
constexpr int CONFIRM_BUTTON_WIDTH = 200;
}  // namespace

CharacterScreen::CharacterScreen(QWidget* parent):
        BaseScreen(parent),
        preview(new CharacterPreview(PREVIEW_SIZE, this)),
        nameInput(new QLineEdit(this)),
        hairStyle(
                new OptionStepper("Peinado", AppearanceCatalog::HAIR_STYLE_COUNT, QString(), this)),
        hairColor(new ColorPicker(this)),
        shirt(new OptionStepper("Camisa", AppearanceCatalog::SHIRT_COUNT, QString(), this)),
        pants(new OptionStepper("Pantalón", AppearanceCatalog::PANTS_COUNT, QString(), this)),
        hat(new OptionStepper("Sombrero", AppearanceCatalog::HAT_COUNT, "Sin sombrero", this)),
        errorLabel(new QLabel(this)),
        confirmButton(new MenuButton("Confirmar", this)) {
    auto* content = new QHBoxLayout();
    content->setSpacing(Theme::SCREEN_SPACING * 3);
    content->addStretch();
    content->addWidget(preview, 0, Qt::AlignVCenter);
    content->addWidget(createForm());
    content->addStretch();

    errorLabel->setObjectName("errorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->setFixedHeight(Theme::ERROR_LABEL_HEIGHT);
    confirmButton->setFixedWidth(CONFIRM_BUTTON_WIDTH);
    // Sin nombre no hay personaje.
    confirmButton->setEnabled(false);

    auto* bottomRow = new QHBoxLayout();
    bottomRow->addWidget(errorLabel, 1);
    bottomRow->addWidget(confirmButton);

    addBackButton();
    mainLayout->addWidget(createTitle("Tu personaje"));
    mainLayout->addSpacing(10);
    mainLayout->addLayout(content, 1);
    mainLayout->addLayout(bottomRow);

    connect(nameInput, &QLineEdit::textChanged, this, [this](const QString& text) {
        confirmButton->setEnabled(!text.trimmed().isEmpty());
        errorLabel->clear();
        refreshPreview();
    });
    connect(nameInput, &QLineEdit::returnPressed, this, &CharacterScreen::confirm);
    for (OptionStepper* stepper: {hairStyle, shirt, pants, hat}) {
        connect(stepper, &OptionStepper::valueChanged, this, &CharacterScreen::refreshPreview);
    }
    connect(hairColor, &ColorPicker::valueChanged, this, &CharacterScreen::refreshPreview);
    connect(confirmButton, &QPushButton::clicked, this, &CharacterScreen::confirm);
}

QWidget* CharacterScreen::createForm() {
    nameInput->setPlaceholderText("Nombre del personaje");
    nameInput->setMaxLength(MAX_NAME_LENGTH);

    auto* form = new QWidget(this);
    form->setFixedWidth(Theme::MENU_COLUMN_WIDTH);
    auto* layout = new QVBoxLayout(form);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::SCREEN_SPACING);
    layout->addWidget(new QLabel("Nombre", form));
    layout->addWidget(nameInput);
    layout->addWidget(hairStyle);
    layout->addWidget(new QLabel("Color de pelo", form));
    layout->addWidget(hairColor);
    layout->addWidget(shirt);
    layout->addWidget(pants);
    layout->addWidget(hat);
    layout->addStretch();
    return form;
}

void CharacterScreen::showEvent(QShowEvent* event) {
    BaseScreen::showEvent(event);
    nameInput->clear();
    for (OptionStepper* stepper: {hairStyle, shirt, pants, hat}) {
        stepper->setValue(0);
    }
    hairColor->setValue(0);
    errorLabel->clear();
    refreshPreview();
    nameInput->setFocus();
}

CharacterAppearance CharacterScreen::currentAppearance() const {
    CharacterAppearance appearance{};
    appearance.hairStyle = static_cast<uint16_t>(hairStyle->value());
    appearance.hairColor = static_cast<uint8_t>(hairColor->value());
    appearance.shirt = static_cast<uint16_t>(shirt->value());
    appearance.pants = static_cast<uint16_t>(pants->value());
    appearance.hat = static_cast<uint16_t>(hat->value());
    return appearance;
}

void CharacterScreen::refreshPreview() {
    preview->setCharacter(currentAppearance(), nameInput->text().trimmed());
}

void CharacterScreen::confirm() {
    const QString name = nameInput->text().trimmed();
    if (name.isEmpty()) {
        return;
    }
    emit characterChosen(name, currentAppearance());
}

void CharacterScreen::showError(LobbyStatus status) {
    switch (status) {
        case LobbyStatus::InvalidName:
            errorLabel->setText("Usá solo letras, números, guion o guion bajo, sin espacios.");
            break;
        case LobbyStatus::NameTaken:
            errorLabel->setText("Ya hay un personaje con ese nombre en la partida.");
            break;
        case LobbyStatus::InvalidAppearance:
            errorLabel->setText("Alguna parte del personaje no es válida. Probá con otra.");
            break;
        case LobbyStatus::GameFull:
            errorLabel->setText("La partida ya tiene el máximo de jugadores.");
            break;
        case LobbyStatus::GameNotFound:
        case LobbyStatus::GameClosed:
            errorLabel->setText("La partida ya no está disponible.");
            break;
        default:
            errorLabel->setText("No se pudo crear el personaje. Intentá de nuevo.");
            break;
    }
}
