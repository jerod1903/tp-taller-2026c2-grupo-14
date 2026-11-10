#include "editor/GameDialog.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "client/menu/components/MenuButton.h"

namespace {
// El ancho del borde de madera del marco
constexpr int FRAME_WIDTH = 14;
constexpr int PADDING = 20;
constexpr int FIELD_SPACING = 16;
// Hay lugar en la ventana: el cartel no se achica a lo justo
constexpr int MIN_WIDTH = 600;

// El borde de madera del marco de las listas del menu; adentro, un panel del marron del menu que
// tapa el centro claro del marco. Los textos toman el color claro de la hoja global y los campos
// el estilo de sus inputs, un poco mas bajos.
const char* const DIALOG_STYLE = R"(
    #gameDialog {
        border-image: url(:/menu/botones/marco_lista.png) 14 stretch;
        border-width: 14px;
    }
    #dialogPanel {
        background-color: #2D1B10;
    }
    #dialogTitle {
        color: #FFEBAD;
        font-size: 20px;
        font-weight: bold;
    }
    #gameDialog QLineEdit {
        border-width: 10px 12px;
        min-height: 44px;
        max-height: 44px;
    }
)";
}  // namespace

GameDialog::GameDialog(const QString& title, const QString& note, QWidget* parent):
        QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint),
        acceptButton(new MenuButton("Aceptar", this)) {
    setObjectName("gameDialog");
    setStyleSheet(DIALOG_STYLE);

    auto* panel = new QFrame(this);
    panel->setObjectName("dialogPanel");
    auto* titleLabel = new QLabel(title, panel);
    titleLabel->setObjectName("dialogTitle");

    acceptButton->setSmall();
    acceptButton->setDefault(true);
    auto* cancel = new MenuButton("Cancelar", this);
    cancel->setSmall();
    auto* buttons = new QHBoxLayout();
    buttons->addWidget(cancel);
    buttons->addWidget(acceptButton);

    fields = new QGridLayout();
    fields->setHorizontalSpacing(FIELD_SPACING);
    fields->setColumnStretch(1, 1);

    auto* content = new QVBoxLayout(panel);
    content->setContentsMargins(PADDING, PADDING, PADDING, PADDING);
    content->setSpacing(PADDING);
    content->addWidget(titleLabel, 0, Qt::AlignHCenter);
    content->addWidget(new QLabel(note, panel));
    content->addLayout(fields);
    content->addLayout(buttons);
    setMinimumWidth(MIN_WIDTH);

    // El panel queda adentro del borde de madera
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH);
    layout->addWidget(panel);

    connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
}

void GameDialog::addField(const QString& label, QWidget* field) {
    const int row = fields->rowCount();
    fields->addWidget(new QLabel(label, this), row, 0, Qt::AlignLeft | Qt::AlignVCenter);
    fields->addWidget(field, row, 1, Qt::AlignLeft | Qt::AlignVCenter);
}

void GameDialog::setAcceptEnabled(bool enabled) { acceptButton->setEnabled(enabled); }
