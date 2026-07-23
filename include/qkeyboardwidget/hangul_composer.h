#pragma once

#include <QObject>
#include <QString>

#include "qkeyboardwidget/qkw_export.h"

namespace qkw {

// Composes individual Hangul jamo (as emitted one-at-a-time by a 2-beolsik
// layout such as layouts/ko.json, via KeyboardController::characterEntered)
// into precomposed Hangul syllable blocks, following the standard Unicode
// Hangul composition algorithm: syllable = 0xAC00 + (cho*21 + jung)*28 + jong.
//
// This is deliberately a standalone helper rather than something wired
// automatically into KeyboardController: composing jamo is a host-side
// concern (it affects how already-inserted text is edited/replaced), and
// non-Hangul layouts must keep behaving exactly as before. A host app opts
// in by feeding KeyboardController::characterEntered/backspaceRequested
// through an instance of this class before touching its text field. See
// examples/widgets_example for a worked example.
//
// Not registered for QML: composition is a text-editing concern that belongs
// wherever the host owns the text field, which for the QML view is host QML
// code, not this library's KeyboardPanel.
class QKW_EXPORT HangulComposer : public QObject
{
    Q_OBJECT

public:
    explicit HangulComposer(QObject *parent = nullptr);

    // Feeds one input token (normally a single character, as delivered by
    // KeyboardController::characterEntered). Returns true if `text` was a
    // single Hangul jamo consumed into composition — the host should not
    // insert `text` itself, only react to syllableReady(). Returns false
    // for anything else (multi-character text, non-jamo characters), after
    // flushing any in-progress composition first — the host should insert
    // `text` verbatim as it would without a composer.
    bool feed(const QString &text);

    // Handles a backspace press. Returns true if it was absorbed by
    // decomposing in-progress state (host should not also remove a
    // character itself — syllableReady()/syllableCleared() already reflects
    // the result). Returns false if nothing was being composed (host should
    // handle backspace as it would without a composer).
    bool backspace();

    // Ends any in-progress composition without changing its last emitted
    // text, so a later feed() starts a fresh syllable. Call this before
    // Enter, on focus-out, or whenever jamo state shouldn't carry over.
    void commit();

    bool isComposing() const;

signals:
    // A composed character is ready. `replacePrevious` is true when this
    // updates the syllable currently being composed (the host should remove
    // the one character it previously inserted for this composition, then
    // insert `text`); false when it starts a brand-new character (the host
    // should just insert `text`).
    void syllableReady(const QString &text, bool replacePrevious);

    // The in-progress composition was fully backspaced away with nothing
    // left; the host should remove the one character it had displayed for
    // it (there is no replacement text).
    void syllableCleared();

private:
    QString composeCurrent() const;
    void startNewSyllable(int choseongIndex);

    int _cho;  // choseong index [0,18], -1 if none
    int _jung; // jungseong index [0,20], -1 if none
    int _jong; // jongseong index [0,27], 0 = no final
};

} // namespace qkw
