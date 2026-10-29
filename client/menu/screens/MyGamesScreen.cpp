#include "client/menu/screens/MyGamesScreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QShowEvent>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "client/menu/backend/ILobbyProtocol.h"
#include "client/menu/components/CharacterPreview.h"
#include "client/menu/components/MenuButton.h"
#include "client/menu/components/SavedGameRow.h"
#include "client/menu/styles/StyleManager.h"

namespace {
constexpr int DETAIL_PREVIEW_SIZE = 96;
constexpr int WORLD_PREVIEW_HEIGHT = 200;
constexpr int PLAY_BUTTON_WIDTH = 200;
}  // namespace

MyGamesScreen::MyGamesScreen(ILobbyProtocol& lobby, QWidget* parent):
        BaseScreen(parent),
        lobby(lobby),
        listOrEmpty(new QStackedWidget(this)),
        gameList(new QListWidget(this)),
        detailOrHint(new QStackedWidget(this)),
        worldPreview(new QLabel(this)),
        detailCharacter(new CharacterPreview(DETAIL_PREVIEW_SIZE, this)),
        detailGameName(new QLabel(this)),
        detailInfo(new QLabel(this)),
        errorLabel(new QLabel(this)),
        playButton(new MenuButton("Jugar", this)) {
    // Derecha: la lista, o un mensaje si todavia no hay partidas.
    auto* emptyMessage = new QLabel("Todavía no tenés partidas.\nCreá una nueva o unite a la de "
                                    "alguien con su código.",
                                    this);
    emptyMessage->setAlignment(Qt::AlignCenter);
    gameList->setObjectName("savedGameList");
    listOrEmpty->addWidget(gameList);
    listOrEmpty->addWidget(emptyMessage);

    auto* content = new QHBoxLayout();
    content->setSpacing(Theme::SCREEN_SPACING * 2);
    content->addWidget(createDetailPanel(), 1);
    content->addWidget(listOrEmpty, 1);

    errorLabel->setObjectName("errorLabel");
    playButton->setFixedWidth(PLAY_BUTTON_WIDTH);
    playButton->setEnabled(false);

    auto* bottomRow = new QHBoxLayout();
    bottomRow->addWidget(errorLabel, 1);
    bottomRow->addWidget(playButton);

    addBackButton();
    mainLayout->addWidget(createTitle("Mis partidas"));
    mainLayout->addSpacing(10);
    mainLayout->addLayout(content, 1);
    mainLayout->addLayout(bottomRow);

    connect(gameList, &QListWidget::currentRowChanged, this, &MyGamesScreen::showDetail);
    // Doble click en una partida es lo mismo que seleccionarla y tocar "Jugar".
    connect(gameList, &QListWidget::itemDoubleClicked, this, &MyGamesScreen::playSelected);
    connect(playButton, &QPushButton::clicked, this, &MyGamesScreen::playSelected);
}

QWidget* MyGamesScreen::createDetailPanel() {
    // Provisorio hasta definir de donde sale la miniatura del mundo
    worldPreview->setObjectName("worldPreview");
    worldPreview->setAlignment(Qt::AlignCenter);
    worldPreview->setFixedHeight(WORLD_PREVIEW_HEIGHT);

    detailGameName->setObjectName("rowTitle");
    detailGameName->setWordWrap(true);
    detailInfo->setWordWrap(true);

    auto* characterRow = new QHBoxLayout();
    characterRow->setSpacing(14);
    characterRow->addWidget(detailCharacter, 0, Qt::AlignTop);
    auto* texts = new QVBoxLayout();
    texts->addWidget(detailGameName);
    texts->addWidget(detailInfo);
    texts->addStretch();
    characterRow->addLayout(texts, 1);

    auto* detail = new QWidget(this);
    auto* detailLayout = new QVBoxLayout(detail);
    detailLayout->setContentsMargins(0, 0, 0, 0);
    detailLayout->setSpacing(Theme::SCREEN_SPACING);
    detailLayout->addWidget(worldPreview);
    detailLayout->addLayout(characterRow);
    detailLayout->addStretch();

    auto* hint = new QLabel("Elegí una partida de la lista para ver su detalle.", this);
    hint->setAlignment(Qt::AlignCenter);
    hint->setWordWrap(true);

    detailOrHint->addWidget(hint);
    detailOrHint->addWidget(detail);
    return detailOrHint;
}

void MyGamesScreen::showEvent(QShowEvent* event) {
    BaseScreen::showEvent(event);
    reloadGames();
}

void MyGamesScreen::reloadGames() {
    games = lobby.listMyGames();
    errorLabel->clear();

    gameList->clear();
    for (const auto& game: games) {
        auto* row = new SavedGameRow(game);
        auto* item = new QListWidgetItem(gameList);
        item->setSizeHint(row->sizeHint());
        gameList->setItemWidget(item, row);
    }

    listOrEmpty->setCurrentIndex(games.empty() ? 1 : 0);
    // Arranca con la primera partida seleccionada (resaltada en la lista y con su detalle).
    if (games.empty()) {
        showDetail(-1);
    } else {
        gameList->setCurrentRow(0);
    }
}

void MyGamesScreen::showDetail(int row) {
    const bool hasSelection = row >= 0 && static_cast<std::size_t>(row) < games.size();
    playButton->setEnabled(hasSelection);
    errorLabel->clear();
    if (!hasSelection) {
        detailOrHint->setCurrentIndex(0);
        return;
    }

    const SavedGameInfo& game = games[static_cast<std::size_t>(row)];
    const QString characterName = QString::fromStdString(game.characterName);
    // SavedGameInfo no trae el mundo; si el server lo agrega, va aca.
    worldPreview->setText("(vista previa pendiente)");
    detailCharacter->setCharacter(game.appearance, characterName);
    detailGameName->setText(QString::fromStdString(game.gameName));
    detailInfo->setText("Personaje: " + characterName + "\n" +
                        (game.isMultiplayer ? "Multijugador" : "Individual"));
    detailOrHint->setCurrentIndex(1);
}

void MyGamesScreen::playSelected() {
    const int row = gameList->currentRow();
    if (row < 0 || static_cast<std::size_t>(row) >= games.size()) {
        return;
    }

    // Retomar es unirse por gameId con el personaje que ya se tiene en esa partida.
    const SavedGameInfo& game = games[static_cast<std::size_t>(row)];
    JoinGameResponse response = lobby.joinGame({game.gameId, game.characterName, game.appearance});
    if (response.status == LobbyStatus::Ok) {
        emit gameEntered(game.gameId);
    } else {
        errorLabel->setText("No se pudo cargar la partida. Probá de nuevo.");
    }
}
