#include "qkeyboardwidget/keyboard_widget.h"

#include "qkeyboardwidget/key_button.h"
#include "qkeyboardwidget/keyboard_controller.h"
#include "qkeyboardwidget/keyboard_theme.h"

#include <QGridLayout>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QVariantMap>

namespace qkw {

KeyboardWidget::KeyboardWidget(QWidget *parent) :
    QWidget(parent),
    _controller(new KeyboardController(this)),
    _theme(new KeyboardTheme(this)),
    _stack(new QStackedWidget(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_stack);

    connect(_controller, &KeyboardController::layoutChanged, this, &KeyboardWidget::rebuildPages);
    connect(_controller, &KeyboardController::currentPageChanged, this, &KeyboardWidget::showCurrentPage);
    connect(_theme, &KeyboardTheme::changed, this, &KeyboardWidget::applyThemeToAllKeys);
}

KeyboardWidget::~KeyboardWidget() = default;

KeyboardController *KeyboardWidget::controller() const
{
    return _controller;
}

KeyboardTheme *KeyboardWidget::theme() const
{
    return _theme;
}

void KeyboardWidget::rebuildPages()
{
    while (_stack->count() > 0) {
        QWidget *page = _stack->widget(0);
        _stack->removeWidget(page);
        page->deleteLater();
    }
    _buttonsByPage.clear();

    if (!_controller->isValid()) return;

    const int pageCount = _controller->pageCount();
    _buttonsByPage.resize(pageCount);

    for (int p = 0; p < pageCount; ++p) {
        auto *pageWidget = new QWidget(_stack);
        auto *grid = new QGridLayout(pageWidget);
        grid->setSpacing(static_cast<int>(_theme->keySpacing()));

        const QVariantList rows = _controller->rowsForPage(p);
        QVector<QVector<KeyButton *>> pageButtons;
        pageButtons.reserve(rows.size());

        for (int r = 0; r < rows.size(); ++r) {
            QVector<KeyButton *> rowButtons;
            const QVariantList row = rows.at(r).toList();
            int column = 0;
            for (const QVariant &keyVariant : row) {
                const QVariantMap key = keyVariant.toMap();
                const QString text = key.value(QStringLiteral("text")).toString();
                const QString icon = key.value(QStringLiteral("icon")).toString();
                const int span = qMax(1, key.value(QStringLiteral("span")).toInt());
                const int row_ = key.value(QStringLiteral("row")).toInt();
                const int col_ = key.value(QStringLiteral("column")).toInt();

                auto *button = new KeyButton(text, icon, pageWidget);
                grid->addWidget(button, r, column, 1, span);
                connect(button, &QPushButton::clicked, _controller,
                        [this, row_, col_]() { _controller->activateKeyAt(row_, col_); });

                rowButtons.append(button);
                column += span;
            }
            pageButtons.append(rowButtons);
        }
        _buttonsByPage[p] = pageButtons;
        _stack->addWidget(pageWidget);
    }

    applyThemeToAllKeys();
    showCurrentPage();
}

void KeyboardWidget::showCurrentPage()
{
    if (!_controller->isValid()) return;
    _stack->setCurrentIndex(_controller->currentPageIndex());
}

void KeyboardWidget::applyThemeToAllKeys()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, _theme->backgroundColor());
    setAutoFillBackground(true);
    setPalette(pal);

    for (const auto &page : _buttonsByPage) {
        for (const auto &row : page) {
            for (KeyButton *button : row) {
                button->applyTheme(*_theme);
            }
        }
    }
}

} // namespace qkw
