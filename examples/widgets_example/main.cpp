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
    QApplication app(argc, argv);

    auto *window = new QMainWindow;
    auto *central = new QWidget(window);
    auto *layout = new QVBoxLayout(central);

    auto *localeBox = new QComboBox(central);
    localeBox->addItem(QStringLiteral("English"), QVariant::fromValue(qkw::KeyboardController::Locale::English));
    localeBox->addItem(QStringLiteral("한국어"), QVariant::fromValue(qkw::KeyboardController::Locale::Korean));

    auto *lineEdit = new QLineEdit(central);
    lineEdit->setPlaceholderText(QStringLiteral("Type using the keyboard below"));

    // KeyboardWidget already has KeyboardController::Locale::English loaded
    // and a default dark theme as soon as it's constructed - only need to
    // touch the theme here because this demo wants a different look.
    auto *keyboard = new qkw::KeyboardWidget(central);
    keyboard->theme()->setKeyColor(QColor(QStringLiteral("#2b2b2e")));
    keyboard->theme()->setAccentKeyColor(QColor(QStringLiteral("#ff9500")));
    keyboard->theme()->setCornerRadius(10);

    // HangulComposer turns the individual jamo that resources/layouts/ko.json emits
    // one keystroke at a time into precomposed syllable blocks (see #9).
    // It's a no-op pass-through for any other layout: feed() only consumes
    // Hangul jamo and otherwise just flushes and returns false, so wiring it
    // in unconditionally is safe regardless of which locale is active.
    auto *hangulComposer = new qkw::HangulComposer(central);

    // HangulComposer::replacePrevious assumes the cursor is still sitting
    // right after the last character it emitted. `editingComposedText`
    // flags the insert()/backspace() calls this file makes on the
    // composer's behalf, so the cursorPositionChanged handler below can
    // tell those apart from a cursor move the composer doesn't know about
    // (a click or arrow-key press) and commit() the pending syllable before
    // it can be corrupted (#46). Captured by reference: main()'s frame, and
    // therefore this variable, outlives app.exec() and every connection
    // made against it.
    bool editingComposedText = false;

    QObject::connect(keyboard->controller(), &qkw::KeyboardController::characterEntered, lineEdit,
                     [lineEdit, hangulComposer](const QString &text) {
                         if (!hangulComposer->feed(text)) lineEdit->insert(text);
                     });
    QObject::connect(hangulComposer, &qkw::HangulComposer::syllableReady, lineEdit,
                     [lineEdit, &editingComposedText](const QString &text, bool replacePrevious) {
                         editingComposedText = true;
                         if (replacePrevious) lineEdit->backspace();
                         lineEdit->insert(text);
                         editingComposedText = false;
                     });
    QObject::connect(hangulComposer, &qkw::HangulComposer::syllableCleared, lineEdit,
                     [lineEdit, &editingComposedText]() {
                         editingComposedText = true;
                         lineEdit->backspace();
                         editingComposedText = false;
                     });

    // Any cursor move this file didn't itself cause (a click elsewhere in
    // the field, arrow keys, etc.) invalidates the composer's assumption
    // that it's still positioned right after the last emitted syllable, so
    // flush the in-progress composition rather than let a later feed()
    // corrupt unrelated text.
    QObject::connect(lineEdit, &QLineEdit::cursorPositionChanged, hangulComposer,
                     [hangulComposer, &editingComposedText](int, int) {
                         if (!editingComposedText) hangulComposer->commit();
                     });
    // QLineEdit has no dedicated focus-out signal; QApplication::focusChanged
    // covers it (and any other way focus can leave the field).
    QObject::connect(qApp, &QApplication::focusChanged, hangulComposer,
                     [lineEdit, hangulComposer](QWidget *old, QWidget *) {
                         if (old == lineEdit) hangulComposer->commit();
                     });

    QObject::connect(keyboard->controller(), &qkw::KeyboardController::backspaceRequested, lineEdit,
                     [lineEdit, hangulComposer, &editingComposedText]() {
                         editingComposedText = true;
                         if (!hangulComposer->backspace()) lineEdit->backspace();
                         editingComposedText = false;
                     });
    QObject::connect(keyboard->controller(), &qkw::KeyboardController::enterRequested, lineEdit,
                     [lineEdit, hangulComposer]() {
                         hangulComposer->commit();
                         lineEdit->clear();
                     });

    QObject::connect(localeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), keyboard->controller(),
                     [keyboard, localeBox, hangulComposer](int index) {
                         hangulComposer->commit();
                         keyboard->controller()->setLocale(
                             localeBox->itemData(index).value<qkw::KeyboardController::Locale>());
                     });

    layout->addWidget(localeBox);
    layout->addWidget(lineEdit);
    layout->addWidget(keyboard);

    window->setCentralWidget(central);
    window->resize(560, 420);
    window->show();

    return app.exec();
}
