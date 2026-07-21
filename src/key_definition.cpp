#include "qkeyboardwidget/key_definition.h"

namespace qkw {

QVariantMap KeyDefinition::toVariantMap(int row, int column) const
{
    QVariantMap map;
    map[QStringLiteral("row")] = row;
    map[QStringLiteral("column")] = column;
    map[QStringLiteral("action")] = static_cast<int>(action);
    map[QStringLiteral("text")] = text;
    map[QStringLiteral("labelId")] = labelId;
    map[QStringLiteral("icon")] = icon;
    map[QStringLiteral("target")] = target;
    map[QStringLiteral("span")] = span;
    return map;
}

} // namespace qkw
