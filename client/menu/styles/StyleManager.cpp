#include "client/menu/styles/StyleManager.h"

QString StyleManager::globalStyleSheet() {
    // Los sprites de madera (botones, inputs) vienen de los recursos Qt (menu.qrc). Los
    // widgets no pintan fondo propio: el fondo lo pone MainWindow detras de todas las pantallas.
    return R"(
        QMainWindow, QMessageBox {
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
            background: transparent;
        }
        #rowTitle {
            color: #FFEBAD;
            font-size: 18px;
            font-weight: bold;
        }
        #badgeMultiplayer, #badgeSingle {
            font-size: 12px;
            font-weight: bold;
            border-radius: 4px;
            padding: 2px 8px;
        }
        #badgeMultiplayer {
            color: #1E120A;
            background-color: #7FC97F;
        }
        #badgeSingle {
            color: #1E120A;
            background-color: #E8C07A;
        }
        #worldPreview {
            color: #B89B74;
            background-color: #1E120A;
            border: 2px dashed #7B4216;
            border-radius: 6px;
        }
        QListWidget {
            /* Fondo liso y semitransparente, igual que la vista previa. */
            background-color: rgba(30, 18, 10, 190);
            border: 2px solid #7B4216;
            border-radius: 6px;
            outline: none;
        }
        QListWidget::item {
            border-bottom: 1px solid #3E2109;
        }
        QListWidget::item:hover {
            background-color: #3A2415;
        }
        QListWidget::item:selected {
            background-color: #5A310D;
        }
        #worldList {
            color: #F7D290;
            font-size: 16px;
        }
        #worldList::item {
            padding: 10px;
        }
        #worldList::item:selected {
            color: #FFEBAD;
        }
        #savedGameRow, #characterPreview, #optionStepper, #colorPicker {
            background: transparent;
        }
        #colorSwatch {
            border: 2px solid #3E2109;
            border-radius: 4px;
            padding: 0;
            border-image: none;
        }
        #colorSwatch:hover {
            border-color: #F7D290;
        }
        #colorSwatch:checked {
            border: 3px solid #FFEBAD;
        }
        #errorLabel {
            color: #FF9B7A;
            font-size: 14px;
        }
        QLineEdit {
            background: transparent;
            color: #FFF3CD;
            border-image: url(:/menu/botones/input_320.png) 15 12 15 12 stretch;
            border-width: 15px 12px;
            padding: 0px 14px;
            /* Todos los inputs de la app tienen la misma altura. */
            min-height: 56px;
            max-height: 56px;
            font-size: 16px;
        }
        QPushButton {
            background: transparent;
            color: #FFF3CD;
            border-image: url(:/menu/botones/boton_320.png) 32 stretch;
            border-width: 32px;
            font-size: 16px;
            font-weight: bold;
            padding: 0px 24px;
        }
        QPushButton:hover {
            color: #FFEBAD;
        }
        QPushButton:pressed {
            padding-top: 8px;
        }
        QPushButton:disabled {
            border-image: url(:/menu/botones/boton_deshabilitado_320.png) 32 stretch;
            color: #5A3A22;
        }
        /* Boton "Volver": sprite de 140x59, al tamano exacto del widget. */
        #backButton {
            border-image: url(:/menu/botones/boton_volver.png) 0 stretch;
            border-width: 0px;
            font-size: 14px;
            padding: 0px;
        }
        /* Panel de opciones del personaje: mismo estilo flat que la lista de partidas. */
        #stepperPanel {
            background-color: rgba(30, 18, 10, 190);
            border: 2px solid #7B4216;
            border-radius: 6px;
        }
        /* Botones chicos: sprite propio de 200 px, sin escalar. */
        #smallButton {
            border-image: url(:/menu/botones/boton_200.png) 32 stretch;
        }
        #smallButton:disabled {
            border-image: url(:/menu/botones/boton_deshabilitado_200.png) 32 stretch;
        }
        /* Flechas del selector: solo el simbolo, sin recuadro de madera. */
        #arrowButton, #arrowButton:hover, #arrowButton:pressed, #arrowButton:disabled {
            background: transparent;
            border: none;
            border-image: none;
            padding: 0;
        }
    )";
}
