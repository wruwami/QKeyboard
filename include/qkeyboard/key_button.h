#pragma once

#include <QPushButton>

#include "qkeyboard/qkw_export.h"

namespace qkw {

class KeyboardTheme;

// A single visual key. Font size auto-shrinks/grows to fill the button on
// resize, and appearance (colors, corner radius, font) comes entirely from
// a KeyboardTheme rather than being hardcoded, so the whole keyboard can be
// re-skinned by swapping/mutating one theme object.
class QKW_EXPORT KeyButton : public QPushButton
{
    Q_OBJECT

public:
    explicit KeyButton(const QString &text, const QString &iconPath = QString(), QWidget *parent = nullptr);

    // Highlights the key as "active" (e.g. Shift while on the uppercase page).
    bool isAccented() const;
    void setAccented(bool accented);

    void applyTheme(const KeyboardTheme &theme);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void fitFontToButton();
    void fitIconToButton();

    bool _accented = false;
};

} // namespace qkw
