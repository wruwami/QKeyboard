#pragma once

#include <QObject>
#include <QVariantList>

#include "qkeyboardwidget/keyboard_layout.h"

#ifdef QKW_ENABLE_QML
#include <qqmlregistration.h>
#endif

namespace qkw {

// Framework-agnostic keyboard state machine: owns the parsed KeyboardLayout,
// tracks which page is showing, resolves translatable control-key labels,
// and turns key activations into character/backspace/enter signals. Both the
// QWidget view (KeyboardWidget) and the QML view (KeyboardPanel.qml) drive
// themselves purely from this class, so there is exactly one place that
// understands the layout JSON and one place that understands typing logic.
class KeyboardController : public QObject
{
    Q_OBJECT
#ifdef QKW_ENABLE_QML
    QML_ELEMENT
#endif

    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY layoutChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY layoutChanged)
    Q_PROPERTY(QString locale READ locale NOTIFY layoutChanged)
    Q_PROPERTY(int currentPageIndex READ currentPageIndex WRITE setCurrentPageIndex NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentPageId READ currentPageId NOTIFY currentPageChanged)
    Q_PROPERTY(QVariantList rows READ rows NOTIFY currentPageChanged)

public:
    explicit KeyboardController(QObject *parent = nullptr);

    // Loads a layout from a local path or a Qt resource ("qrc:/..." or ":/...").
    Q_INVOKABLE bool loadFile(const QString &filePath);
    Q_INVOKABLE bool loadJson(const QByteArray &json);

    bool isValid() const;
    QString errorString() const;
    QString locale() const;

    QString source() const;
    void setSource(const QString &filePath);

    int currentPageIndex() const;
    void setCurrentPageIndex(int index);
    QString currentPageId() const;
    Q_INVOKABLE void setPageById(const QString &pageId);

    Q_INVOKABLE int pageCount() const;

    // Current page's rows as a list of lists of key maps (row, column, action,
    // text, labelId resolved to a translated display label, icon, target, span).
    QVariantList rows() const;

    // Same shape as rows(), for an arbitrary page index. Used by views (like
    // KeyboardWidget) that pre-build every page up front.
    Q_INVOKABLE QVariantList rowsForPage(int pageIndex) const;

    // Invoked by the QWidget/QML key buttons when the key at (row, column) on
    // the current page is pressed.
    Q_INVOKABLE void activateKeyAt(int row, int column);

signals:
    void sourceChanged();
    void layoutChanged();
    void currentPageChanged();

    void characterEntered(const QString &text);
    void backspaceRequested();
    void enterRequested();

private:
    QString resolveLabel(const KeyDefinition &key) const;

    KeyboardLayout _layout;
    QString _source;
    QString _errorString;
    int _currentPageIndex = 0;
};

} // namespace qkw
