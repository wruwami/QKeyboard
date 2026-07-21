#include "qkeyboardwidget/keyboard_controller.h"

#include <QCoreApplication>

namespace qkw {

KeyboardController::KeyboardController(QObject *parent) : QObject(parent)
{
}

bool KeyboardController::loadFile(const QString &filePath)
{
    QString path = filePath;
    if (path.startsWith(QLatin1String("qrc:")))
        path = path.mid(3);

    QString error;
    KeyboardLayout layout = KeyboardLayout::fromFile(path, &error);
    if (!layout.isValid()) {
        _errorString = error;
        emit layoutChanged();
        return false;
    }

    _layout = layout;
    _source = filePath;
    _errorString.clear();
    _currentPageIndex = 0;
    emit sourceChanged();
    emit layoutChanged();
    emit currentPageChanged();
    return true;
}

bool KeyboardController::loadJson(const QByteArray &json)
{
    QString error;
    KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    if (!layout.isValid()) {
        _errorString = error;
        emit layoutChanged();
        return false;
    }

    _layout = layout;
    _source.clear();
    _errorString.clear();
    _currentPageIndex = 0;
    emit sourceChanged();
    emit layoutChanged();
    emit currentPageChanged();
    return true;
}

bool KeyboardController::isValid() const
{
    return _layout.isValid();
}

QString KeyboardController::errorString() const
{
    return _errorString;
}

QString KeyboardController::locale() const
{
    return _layout.locale();
}

QString KeyboardController::source() const
{
    return _source;
}

void KeyboardController::setSource(const QString &filePath)
{
    if (filePath == _source) return;
    loadFile(filePath);
}

int KeyboardController::currentPageIndex() const
{
    return _currentPageIndex;
}

void KeyboardController::setCurrentPageIndex(int index)
{
    if (!_layout.isValid()) return;
    if (index < 0 || index >= _layout.pages().size()) return;
    if (index == _currentPageIndex) return;
    _currentPageIndex = index;
    emit currentPageChanged();
}

QString KeyboardController::currentPageId() const
{
    if (!_layout.isValid()) return {};
    return _layout.pages().at(_currentPageIndex).id;
}

void KeyboardController::setPageById(const QString &pageId)
{
    const int index = _layout.indexOfPage(pageId);
    if (index < 0) return;
    setCurrentPageIndex(index);
}

int KeyboardController::pageCount() const
{
    return _layout.isValid() ? _layout.pages().size() : 0;
}

QVariantList KeyboardController::rows() const
{
    return rowsForPage(_currentPageIndex);
}

QVariantList KeyboardController::rowsForPage(int pageIndex) const
{
    QVariantList result;
    if (!_layout.isValid()) return result;
    if (pageIndex < 0 || pageIndex >= _layout.pages().size()) return result;

    const KeyboardPage &page = _layout.pages().at(pageIndex);
    for (int r = 0; r < page.rows.size(); ++r) {
        QVariantList rowList;
        const QVector<KeyDefinition> &row = page.rows.at(r);
        for (int c = 0; c < row.size(); ++c) {
            const KeyDefinition &key = row.at(c);
            QVariantMap map = key.toVariantMap(r, c);
            map[QStringLiteral("text")] = resolveLabel(key);
            rowList.append(map);
        }
        result.append(rowList);
    }
    return result;
}

QString KeyboardController::resolveLabel(const KeyDefinition &key) const
{
    if (key.action == KeyAction::Character) return key.text;

    QString labelId = key.labelId;
    if (labelId.isEmpty()) {
        switch (key.action) {
            case KeyAction::Backspace: labelId = QStringLiteral("backspace"); break;
            case KeyAction::Enter: labelId = QStringLiteral("enter"); break;
            case KeyAction::Space: labelId = QStringLiteral("space"); break;
            case KeyAction::Shift: labelId = QStringLiteral("shift"); break;
            default: break;
        }
    }

    // Every recognized id must appear here as a literal QCoreApplication::translate()
    // call (fixed context "QKeyboardWidget") so that `lupdate` can discover it
    // statically and the translation lookup at runtime doesn't depend on how a
    // given Qt version mangles a namespaced class name into a tr() context.
    // Unknown ids fall back to whatever the layout JSON put in "labelId" so a
    // new layout still works (untranslated) without needing a code change.
    static const char *kContext = "QKeyboardWidget";
    if (labelId == QLatin1String("backspace"))
        return QCoreApplication::translate(kContext, "Backspace", "keyboard control key label");
    if (labelId == QLatin1String("enter"))
        return QCoreApplication::translate(kContext, "Enter", "keyboard control key label");
    if (labelId == QLatin1String("space"))
        return QCoreApplication::translate(kContext, "Space", "keyboard control key label");
    if (labelId == QLatin1String("shift"))
        return QCoreApplication::translate(kContext, "Shift", "keyboard control key label");
    if (labelId == QLatin1String("numbers"))
        return QCoreApplication::translate(kContext, "123", "keyboard control key label");
    if (labelId == QLatin1String("letters"))
        return QCoreApplication::translate(kContext, "ABC", "keyboard control key label");
    if (labelId == QLatin1String("symbols"))
        return QCoreApplication::translate(kContext, "#+=", "keyboard control key label");

    return labelId.isEmpty() ? key.text : labelId;
}

void KeyboardController::activateKeyAt(int row, int column)
{
    if (!_layout.isValid()) return;
    const KeyboardPage &page = _layout.pages().at(_currentPageIndex);
    if (row < 0 || row >= page.rows.size()) return;
    const QVector<KeyDefinition> &rowVec = page.rows.at(row);
    if (column < 0 || column >= rowVec.size()) return;

    const KeyDefinition &key = rowVec.at(column);
    switch (key.action) {
        case KeyAction::Character:
            emit characterEntered(key.text);
            break;
        case KeyAction::Space:
            emit characterEntered(QStringLiteral(" "));
            break;
        case KeyAction::Backspace:
            emit backspaceRequested();
            break;
        case KeyAction::Enter:
            emit enterRequested();
            break;
        case KeyAction::Shift:
        case KeyAction::Switch:
            setPageById(key.target);
            break;
    }
}

} // namespace qkw
