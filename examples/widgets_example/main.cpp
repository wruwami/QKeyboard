#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>

#include "qkeyboardwidget/hangul_composer.h"
#include "qkeyboardwidget/keyboard_controller.h"
#include "qkeyboardwidget/keyboard_theme.h"
#include "qkeyboardwidget/keyboard_widget.h"

// Minimal demo: a QLineEdit driven entirely by a KeyboardWidget, with a
// combo box that swaps the active layout (and therefore language) at
// runtime, plus a small theme override to show re-skinning.
int main(int argc, char *argv[])
{
    // qkeyboardwidget's layouts/icons are compiled into its static library
    // via resources/qkeyboardwidget.qrc; a static library's resource
    // initializer isn't otherwise referenced by the linker, so it must be
    // forced in explicitly here.
    Q_INIT_RESOURCE(qkeyboardwidget);

    QApplication app(argc, argv);

    auto *window = new QMainWindow;
    auto *central = new QWidget(window);
    auto *layout = new QVBoxLayout(central);

    auto *localeBox = new QComboBox(central);
    localeBox->addItem(QStringLiteral("English"), QStringLiteral(":/layouts/en.json"));
    localeBox->addItem(QStringLiteral("한국어"), QStringLiteral(":/layouts/ko.json"));

    auto *lineEdit = new QLineEdit(central);
    lineEdit->setPlaceholderText(QStringLiteral("Type using the keyboard below"));

    auto *keyboard = new qkw::KeyboardWidget(central);
    keyboard->controller()->loadFile(QStringLiteral(":/layouts/en.json"));
    keyboard->theme()->setKeyColor(QColor(QStringLiteral("#2b2b2e")));
    keyboard->theme()->setAccentKeyColor(QColor(QStringLiteral("#ff9500")));
    keyboard->theme()->setCornerRadius(10);

    auto *composer = new qkw::HangulComposer(keyboard->controller(), central);

    QObject::connect(composer, &qkw::HangulComposer::characterEntered, lineEdit,
                      [lineEdit](const QString &text) { lineEdit->insert(text); });
    QObject::connect(composer, &qkw::HangulComposer::backspaceRequested, lineEdit,
                      [lineEdit]() { lineEdit->backspace(); });
    QObject::connect(keyboard->controller(), &qkw::KeyboardController::enterRequested, lineEdit,
                      [lineEdit, composer]() {
                          composer->commit();
                          lineEdit->clear();
                      });

    QObject::connect(localeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), keyboard->controller(),
                      [keyboard, localeBox](int index) {
                          keyboard->controller()->loadFile(localeBox->itemData(index).toString());
                      });

    layout->addWidget(localeBox);
    layout->addWidget(lineEdit);
    layout->addWidget(keyboard);

    window->setCentralWidget(central);
    window->resize(560, 420);
    window->show();

    return app.exec();
}
