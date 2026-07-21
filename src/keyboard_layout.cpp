#include "qkeyboardwidget/keyboard_layout.h"

#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

namespace qkw {

namespace {

bool actionFromString(const QString &type, KeyAction *outAction)
{
    static const QHash<QString, KeyAction> kActions = {
        {QStringLiteral("char"), KeyAction::Character},
        {QStringLiteral("backspace"), KeyAction::Backspace},
        {QStringLiteral("enter"), KeyAction::Enter},
        {QStringLiteral("space"), KeyAction::Space},
        {QStringLiteral("shift"), KeyAction::Shift},
        {QStringLiteral("switch"), KeyAction::Switch},
    };
    const auto it = kActions.constFind(type);
    if (it == kActions.constEnd()) return false;
    *outAction = it.value();
    return true;
}

bool parseKey(const QJsonValue &value, KeyDefinition *outKey, QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("each key entry must be a JSON object");
        return false;
    }
    const QJsonObject obj = value.toObject();

    const QString type = obj.value(QStringLiteral("type")).toString(QStringLiteral("char"));
    KeyAction action;
    if (!actionFromString(type, &action)) {
        *errorMessage = QStringLiteral("unknown key type '%1'").arg(type);
        return false;
    }

    KeyDefinition key;
    key.action = action;
    key.text = obj.value(QStringLiteral("text")).toString();
    key.labelId = obj.value(QStringLiteral("labelId")).toString();
    key.icon = obj.value(QStringLiteral("icon")).toString();
    key.target = obj.value(QStringLiteral("target")).toString();
    key.span = obj.value(QStringLiteral("span")).toInt(1);

    if (action == KeyAction::Character && key.text.isEmpty()) {
        *errorMessage = QStringLiteral("a 'char' key requires non-empty 'text'");
        return false;
    }
    if ((action == KeyAction::Shift || action == KeyAction::Switch) && key.target.isEmpty()) {
        *errorMessage = QStringLiteral("a 'shift'/'switch' key requires a 'target' page id");
        return false;
    }

    *outKey = key;
    return true;
}

bool parsePage(const QJsonValue &value, KeyboardPage *outPage, QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("each page entry must be a JSON object");
        return false;
    }
    const QJsonObject obj = value.toObject();

    KeyboardPage page;
    page.id = obj.value(QStringLiteral("id")).toString();
    if (page.id.isEmpty()) {
        *errorMessage = QStringLiteral("a page requires a non-empty 'id'");
        return false;
    }

    const QJsonArray rowsArray = obj.value(QStringLiteral("rows")).toArray();
    for (const QJsonValue &rowValue : rowsArray) {
        if (!rowValue.isArray()) {
            *errorMessage = QStringLiteral("each row in page '%1' must be a JSON array").arg(page.id);
            return false;
        }
        QVector<KeyDefinition> row;
        for (const QJsonValue &keyValue : rowValue.toArray()) {
            KeyDefinition key;
            if (!parseKey(keyValue, &key, errorMessage)) return false;
            row.append(key);
        }
        page.rows.append(row);
    }

    *outPage = page;
    return true;
}

} // namespace

KeyboardLayout KeyboardLayout::fromJson(const QByteArray &json, QString *errorMessage)
{
    KeyboardLayout layout;

    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage) *errorMessage = parseError.errorString();
        return layout;
    }
    if (!document.isObject()) {
        if (errorMessage) *errorMessage = QStringLiteral("root of layout JSON must be an object");
        return layout;
    }

    const QJsonObject root = document.object();
    layout._locale = root.value(QStringLiteral("locale")).toString();

    const QJsonArray pagesArray = root.value(QStringLiteral("pages")).toArray();
    if (pagesArray.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("layout must declare at least one page");
        return KeyboardLayout();
    }

    QVector<KeyboardPage> pages;
    QString localError;
    for (const QJsonValue &pageValue : pagesArray) {
        KeyboardPage page;
        if (!parsePage(pageValue, &page, &localError)) {
            if (errorMessage) *errorMessage = localError;
            return KeyboardLayout();
        }
        pages.append(page);
    }
    layout._pages = pages;

    return layout;
}

KeyboardLayout KeyboardLayout::fromFile(const QString &filePath, QString *errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return KeyboardLayout();
    }
    return fromDevice(file, errorMessage);
}

KeyboardLayout KeyboardLayout::fromDevice(QIODevice &device, QString *errorMessage)
{
    if (!device.isReadable() && !device.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = QStringLiteral("layout device is not readable");
        return KeyboardLayout();
    }
    return fromJson(device.readAll(), errorMessage);
}

bool KeyboardLayout::isValid() const
{
    return !_pages.isEmpty();
}

QString KeyboardLayout::locale() const
{
    return _locale;
}

const QVector<KeyboardPage> &KeyboardLayout::pages() const
{
    return _pages;
}

int KeyboardLayout::indexOfPage(const QString &pageId) const
{
    for (int i = 0; i < _pages.size(); ++i) {
        if (_pages.at(i).id == pageId) return i;
    }
    return -1;
}

} // namespace qkw
