#include "client/menu/styles/StyleManager.h"

QString StyleManager::globalStyleSheet() {
    return R"(
        QMainWindow, QWidget {
            background-color: #2D1B10;
        }
        #titleLabel {
            color: #FFEBAD;
            font-size: 32px;
            font-weight: bold;
        }
        QLabel {
            color: #F7D290;
            font-size: 16px;
        }
        #errorLabel {
            color: #FF9B7A;
            font-size: 14px;
        }
        QLineEdit {
            background-color: #1E120A;
            color: #FFF3CD;
            border: 2px solid #7B4216;
            border-radius: 6px;
            padding: 8px;
            font-size: 14px;
        }
        QPushButton {
            background-color: #7B4216;
            color: #FFF3CD;
            border: 3px solid #3E2109;
            border-radius: 8px;
            font-size: 16px;
            font-weight: bold;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #9C551F;
            border-color: #5A310D;
        }
        QPushButton:pressed {
            background-color: #572D0B;
            border-color: #211004;
        }
        QPushButton:disabled {
            background-color: #4A3324;
            color: #8C7A66;
        }
    )";
}
