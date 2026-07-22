#pragma once

#include <QString>
#include <QVariantMap>

namespace qkw {

enum class KeyAction { Character, Backspace, Enter, Space, Shift, Switch };

// A single key on a keyboard page: what it types (for Character) and/or
// which page it navigates to (for Shift/Switch), plus display hints.
struct KeyDefinition
{
    KeyAction action = KeyAction::Character;
    QString text;    // literal character(s) inserted for KeyAction::Character
    QString labelId; // translatable label id for control keys (tr() lookup key)
    QString icon;    // optional icon resource/file path
    QString target;  // target page id for Shift/Switch
    int span = 1;    // column span in the grid

    QVariantMap toVariantMap(int row, int column) const;
};

} // namespace qkw
