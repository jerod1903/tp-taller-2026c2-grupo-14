#include "client/menu/screens/PlaceholderScreen.h"

#include <QLabel>
#include <QVBoxLayout>

PlaceholderScreen::PlaceholderScreen(const QString& title, QWidget* parent): BaseScreen(parent) {
    auto* message = new QLabel("Próximamente", this);
    message->setAlignment(Qt::AlignCenter);

    addBackButton();
    mainLayout->addStretch();
    setTitle(title);
    mainLayout->addWidget(message);
    mainLayout->addStretch();
}
