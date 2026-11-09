#include "editor/FloorSizeDialog.h"

#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "client/menu/components/MenuButton.h"

namespace {
// Lo justo para la pared, la salida, la escalera y algo de piso; mas de 30 es tedioso de editar
constexpr int MIN_SIDE = 5;
constexpr int MAX_SIDE = 30;
// El tamano de los pisos de valle.toml
constexpr int DEFAULT_WIDTH = 20;
constexpr int DEFAULT_HEIGHT = 15;
// El ancho del borde de madera del marco
constexpr int FRAME_WIDTH = 14;
constexpr int PADDING = 16;
constexpr int INPUT_WIDTH = 120;

// El borde de madera del marco de las listas del menu; adentro, un panel del marron del menu que
// tapa el centro claro del marco. Los textos toman el color claro de la hoja global y los campos
// el estilo de sus inputs, un poco mas bajos.
const char* const DIALOG_STYLE = R"(
    #floorSizeDialog {
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
    #floorSizeDialog QLineEdit {
        border-width: 10px 12px;
        min-height: 44px;
        max-height: 44px;
    }
)";

QLineEdit* sideInput(int value, QWidget* parent) {
    auto* input = new QLineEdit(QString::number(value), parent);
    input->setValidator(new QIntValidator(MIN_SIDE, MAX_SIDE, input));
    input->setFixedWidth(INPUT_WIDTH);
    return input;
}
}  // namespace

FloorSizeDialog::FloorSizeDialog(QWidget* parent):
        QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint),
        widthInput(sideInput(DEFAULT_WIDTH, this)),
        heightInput(sideInput(DEFAULT_HEIGHT, this)),
        acceptButton(new MenuButton("Aceptar", this)) {
    setObjectName("floorSizeDialog");
    setStyleSheet(DIALOG_STYLE);

    auto* panel = new QFrame(this);
    panel->setObjectName("dialogPanel");
    auto* title = new QLabel("Pisos de la mina", panel);
    title->setObjectName("dialogTitle");
    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->addRow("Ancho (5 a 30)", widthInput);
    form->addRow("Alto (5 a 30)", heightInput);

    acceptButton->setSmall();
    acceptButton->setDefault(true);
    auto* cancel = new MenuButton("Cancelar", this);
    cancel->setSmall();
    auto* buttons = new QHBoxLayout();
    buttons->addWidget(cancel);
    buttons->addWidget(acceptButton);

    auto* content = new QVBoxLayout(panel);
    content->setContentsMargins(PADDING, PADDING, PADDING, PADDING);
    content->addWidget(title, 0, Qt::AlignHCenter);
    content->addWidget(new QLabel("Todos los pisos miden lo mismo y despues no cambia.", panel));
    content->addLayout(form);
    content->addLayout(buttons);

    // El panel queda adentro del borde de madera
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH);
    layout->addWidget(panel);

    connect(widthInput, &QLineEdit::textChanged, this, &FloorSizeDialog::updateAccept);
    connect(heightInput, &QLineEdit::textChanged, this, &FloorSizeDialog::updateAccept);
    connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
}

FloorSize FloorSizeDialog::size() const {
    return {widthInput->text().toInt(), heightInput->text().toInt()};
}

void FloorSizeDialog::updateAccept() {
    acceptButton->setEnabled(widthInput->hasAcceptableInput() && heightInput->hasAcceptableInput());
}
