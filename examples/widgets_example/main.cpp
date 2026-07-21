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

    // HangulComposer turns the individual jamo that layouts/ko.json emits
    // one keystroke at a time into precomposed syllable blocks (see #9).
    // It's a no-op pass-through for any other layout: feed() only consumes
    // Hangul jamo and otherwise just flushes and returns false, so wiring it
    // in unconditionally is safe regardless of which locale is active.
    auto *hangulComposer = new qkw::HangulComposer(central);

    QObject::connect(keyboard->controller(), &qkw::KeyboardController::characterEntered, lineEdit,
                      [lineEdit, hangulComposer](const QString &text) {
                          if (!hangulComposer->feed(text)) lineEdit->insert(text);
                      });
    QObject::connect(hangulComposer, &qkw::HangulComposer::syllableReady, lineEdit,
                      [lineEdit](const QString &text, bool replacePrevious) {
                          if (replacePrevious) lineEdit->backspace();
                          lineEdit->insert(text);
                      });
    QObject::connect(hangulComposer, &qkw::HangulComposer::syllableCleared, lineEdit,
                      [lineEdit]() { lineEdit->backspace(); });

    QObject::connect(keyboard->controller(), &qkw::KeyboardController::backspaceRequested, lineEdit,
                      [lineEdit, hangulComposer]() {
                          if (!hangulComposer->backspace()) lineEdit->backspace();
                      });
    QObject::connect(keyboard->controller(), &qkw::KeyboardController::enterRequested, lineEdit,
                      [lineEdit, hangulComposer]() {
                          hangulComposer->commit();
                          lineEdit->clear();
                      });

    QObject::connect(localeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), keyboard->controller(),
                      [keyboard, localeBox, hangulComposer](int index) {
                          hangulComposer->commit();
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
