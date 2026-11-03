#include "client/menu/components/SavedGameRow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "client/menu/components/CharacterPreview.h"

namespace {
constexpr int ROW_PREVIEW_SIZE = 56;
}  // namespace

SavedGameRow::SavedGameRow(const SavedGameInfo& game, QWidget* parent): QWidget(parent) {
    setObjectName("savedGameRow");

    const QString characterName = QString::fromStdString(game.characterName);
    auto* preview = new CharacterPreview(QSize(ROW_PREVIEW_SIZE, ROW_PREVIEW_SIZE), this);
    preview->setCharacter(game.appearance, characterName);

    auto* gameName = new QLabel(QString::fromStdString(game.gameName), this);
    gameName->setObjectName("rowTitle");
    auto* character = new QLabel("Personaje: " + characterName, this);
    auto* mode = new QLabel(game.isMultiplayer ? "Multijugador" : "Individual", this);
    mode->setObjectName(game.isMultiplayer ? "badgeMultiplayer" : "badgeSingle");

    auto* texts = new QVBoxLayout();
    texts->setSpacing(2);
    texts->addWidget(gameName);
    texts->addWidget(character);
    texts->addWidget(mode, 0, Qt::AlignLeft);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(14);
    layout->addWidget(preview);
    layout->addLayout(texts, 1);
}
