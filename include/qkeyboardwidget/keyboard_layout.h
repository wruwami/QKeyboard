#pragma once

#include <QString>
#include <QVector>

#include "qkeyboardwidget/key_definition.h"
#include "qkeyboardwidget/qkw_export.h"

class QIODevice;

namespace qkw {

struct QKW_EXPORT KeyboardPage
{
    QString id;
    QVector<QVector<KeyDefinition>> rows;
};

// Parses the qkeyboardwidget layout JSON schema:
//
// {
//   "locale": "en",
//   "pages": [
//     { "id": "lower", "rows": [ [ {"type":"char","text":"q"}, ... ], ... ] },
//     ...
//   ]
// }
//
// This is a from-scratch schema (pages of self-describing key objects)
// rather than the original project's parallel string-array + modifier-map
// design, so each key fully describes its own behavior in place.
class QKW_EXPORT KeyboardLayout
{
public:
    KeyboardLayout() = default;

    static KeyboardLayout fromJson(const QByteArray &json, QString *errorMessage = nullptr);
    static KeyboardLayout fromFile(const QString &filePath, QString *errorMessage = nullptr);
    static KeyboardLayout fromDevice(QIODevice &device, QString *errorMessage = nullptr);

    bool isValid() const;

    QString locale() const;
    const QVector<KeyboardPage> &pages() const;
    int indexOfPage(const QString &pageId) const;

private:
    QString _locale;
    QVector<KeyboardPage> _pages;
};

} // namespace qkw
