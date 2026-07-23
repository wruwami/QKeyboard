#pragma once

#include <QWidget>
#include <QVector>

#include "qkeyboardwidget/qkw_export.h"

class QStackedWidget;
class QGridLayout;

namespace qkw {

class KeyboardController;
class KeyboardTheme;
class KeyButton;

// QWidget-based on-screen keyboard view. Builds one page (a grid of
// KeyButton) per KeyboardLayout page and swaps the visible one when the
// controller's current page changes; nothing here parses JSON or knows
// typing semantics, that all lives in KeyboardController.
class QKW_EXPORT KeyboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardWidget(QWidget *parent = nullptr);
    ~KeyboardWidget() override;

    // Both are owned by this widget and created empty; call
    // controller()->loadFile(...) and mutate theme() to configure them.
    KeyboardController *controller() const;
    KeyboardTheme *theme() const;

private slots:
    void rebuildPages();
    void showCurrentPage();
    void applyThemeToAllKeys();

private:
    KeyboardController *_controller;
    KeyboardTheme *_theme;
    QStackedWidget *_stack;
    QVector<QVector<QVector<KeyButton *>>> _buttonsByPage; // [page][row][column]
};

} // namespace qkw
