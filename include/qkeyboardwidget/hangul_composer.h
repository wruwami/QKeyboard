#pragma once

#include <QObject>
#include <QString>

#include "qkeyboardwidget/abstract_composer.h"
#include "qkeyboardwidget/qkw_export.h"

namespace qkw {

// Composes individual Hangul jamo (as emitted one-at-a-time by a 2-beolsik
// layout such as resources/layouts/ko.json, via KeyboardController::characterEntered)
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
class QKW_EXPORT HangulComposer : public AbstractComposer
{
    Q_OBJECT

public:
    explicit HangulComposer(QObject *parent = nullptr);

    bool feed(const QString &text) override;
    bool backspace() override;
    void reset() override;

    void commit();
    bool isComposing() const override;

    // syllableReady(text, replacePrevious)/syllableCleared() are inherited
    // from AbstractComposer, not redeclared here: Qt signals aren't virtual,
    // so a same-name/same-signature redeclaration in a subclass doesn't
    // override the base signal, it *shadows* it with a second, distinct
    // signal - emit syllableReady(...) inside this class's own methods would
    // then fire HangulComposer::syllableReady while code holding an
    // AbstractComposer* (e.g. via a QSignalSpy on
    // &AbstractComposer::syllableReady) is connected to
    // AbstractComposer::syllableReady and would never see it. Every
    // `&HangulComposer::syllableReady`/`&HangulComposer::syllableCleared`
    // connection elsewhere (examples/widgets_example/main.cpp,
    // tests/tst_hangulcomposer.cpp) still resolves correctly through
    // inheritance without this redeclaration.

private:
    QString composeCurrent() const;
    void startNewSyllable(int choseongIndex);

    int _cho;  // choseong index [0,18], -1 if none
    int _jung; // jungseong index [0,20], -1 if none
    int _jong; // jongseong index [0,27], 0 = no final
};

} // namespace qkw
