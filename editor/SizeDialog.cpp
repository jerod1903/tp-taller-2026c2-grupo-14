#include "editor/SizeDialog.h"

#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "client/menu/components/MenuButton.h"

namespace {
// El ancho del borde de madera del marco
constexpr int FRAME_WIDTH = 14;
constexpr int PADDING = 16;
constexpr int INPUT_WIDTH = 120;

// El borde de madera del marco de las listas del menu; adentro, un panel del marron del menu que
// tapa el centro claro del marco. Los textos toman el color claro de la hoja global y los campos
// el estilo de sus inputs, un poco mas bajos.
const char* const DIALOG_STYLE = R"(
    #sizeDialog {
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
    #sizeDialog QLineEdit {
        border-width: 10px 12px;
        min-height: 44px;
        max-height: 44px;
    }
)";

QLineEdit* sideInput(const SizeRequest& request, int value, QWidget* parent) {
    auto* input = new QLineEdit(QString::number(value), parent);
    input->setValidator(new QIntValidator(request.minSide, request.maxSide, input));
    input->setFixedWidth(INPUT_WIDTH);
    return input;
}
}  // namespace

SizeDialog::SizeDialog(const SizeRequest& request, QWidget* parent):
        QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint),
        widthInput(sideInput(request, request.initial.width, this)),
        heightInput(sideInput(request, request.initial.height, this)),
        acceptButton(new MenuButton("Aceptar", this)) {
    setObjectName("sizeDialog");
    setStyleSheet(DIALOG_STYLE);

    auto* panel = new QFrame(this);
    panel->setObjectName("dialogPanel");
    auto* title = new QLabel(request.title, panel);
    title->setObjectName("dialogTitle");
    const QString range = QString(" (%1 a %2)").arg(request.minSide).arg(request.maxSide);
    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->addRow("Ancho" + range, widthInput);
    form->addRow("Alto" + range, heightInput);

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
    content->addWidget(new QLabel(request.note, panel));
    content->addLayout(form);
    content->addLayout(buttons);

    // El panel queda adentro del borde de madera
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH);
    layout->addWidget(panel);

    connect(widthInput, &QLineEdit::textChanged, this, &SizeDialog::updateAccept);
    connect(heightInput, &QLineEdit::textChanged, this, &SizeDialog::updateAccept);
    connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
}

AreaSize SizeDialog::size() const {
    return {widthInput->text().toInt(), heightInput->text().toInt()};
}

void SizeDialog::updateAccept() {
    acceptButton->setEnabled(widthInput->hasAcceptableInput() && heightInput->hasAcceptableInput());
}
