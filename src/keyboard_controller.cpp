#include "qkeyboardwidget/keyboard_controller.h"

#include <QCoreApplication>
#include <QDebug>

// Q_INIT_RESOURCE() expands to an `extern` declaration of the rcc-generated
// (un-namespaced) initializer function and must therefore be invoked from
// outside any C++ namespace - calling it from within `namespace qkw` would
// instead declare and look up qkw::qInitResources_qkeyboardwidget(), which
// doesn't exist, and fail to link. This wrapper is the one place that calls
// it; KeyboardController's constructor below just calls this ordinary
// function, which has no such restriction.
//
// Needed because qkeyboardwidget's layouts/icons are compiled into this
// static library's own resources/qkeyboardwidget.qrc: the linker only pulls
// the generated resource-initializer object file into a consumer's binary
// if something actually references it, so without this, a statically-linked
// KeyboardController's own default-locale auto-load below would silently
// fail to find ":/layouts/en.json" in any app that never happened to
// initialize the resource itself (as every example/test previously had to
// do explicitly).
static void qkwInitBundledResources()
{
    Q_INIT_RESOURCE(qkeyboardwidget);
}

namespace qkw {

KeyboardController::KeyboardController(QObject *parent) : QObject(parent)
{
    qkwInitBundledResources();
    // Only fails if the bundled resource itself is missing/corrupted (a
    // broken build/deployment, not a normal runtime condition, and not
    // reproducible without actually shipping a broken qkeyboardwidget.qrc)
    // - isValid() and errorString() still correctly reflect that failure
    // afterward (same "leaves observable state behind" contract
    // loadFile()/loadJson() always have), but nothing could have connected
    // to loadFailed() yet this early, so warn too rather than leave it
    // purely silent.
    // LCOV_EXCL_START
    if (!setLocale(Locale::English)) {
        qWarning("KeyboardController: failed to load its default locale: %s", qPrintable(_errorString));
    }
    // LCOV_EXCL_STOP
}

bool KeyboardController::setLocale(Locale locale)
{
    switch (locale) {
        case Locale::English: return loadFile(QStringLiteral(":/layouts/en.json"));
        case Locale::Korean: return loadFile(QStringLiteral(":/layouts/ko.json"));
    }
    // Every Locale enumerator has a case above, so this is genuinely
    // unreachable through the public API (the parameter type itself rules
    // out any other value) - only exists to satisfy -Wreturn-type, which
    // GCC still raises here despite the switch being exhaustive over every
    // enumerator. LCOV_EXCL_LINE: excluded from coverage for the same
    // reason, matching how this codebase already treats other genuinely
    // unreachable defensive lines (e.g. resolveLabel()'s "default: break;").
    Q_UNREACHABLE(); // LCOV_EXCL_LINE
}

bool KeyboardController::loadFile(const QString &filePath)
{
    QString path = filePath;
    if (path.startsWith(QLatin1String("qrc:"))) path = path.mid(3);

    QString error;
    KeyboardLayout layout = KeyboardLayout::fromFile(path, &error);
    if (!layout.isValid()) {
        _errorString = error;
        emit layoutChanged();
        emit loadFailed(error);
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
        emit loadFailed(error);
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
        result.append(QVariant(rowList));
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
        case KeyAction::Character: emit characterEntered(key.text); break;
        case KeyAction::Space: emit characterEntered(QStringLiteral(" ")); break;
        case KeyAction::Backspace: emit backspaceRequested(); break;
        case KeyAction::Enter: emit enterRequested(); break;
        case KeyAction::Shift:
        case KeyAction::Switch: setPageById(key.target); break;
    }
}

} // namespace qkw
