# QKeyboard

An on-screen keyboard widget for Qt, licensed under Apache-2.0. It supports
both QWidget and QML (Qt Quick) applications from the same core, drives its
key layout entirely from JSON (so adding a language or remapping keys never
requires touching code), and exposes a live theme object for re-skinning at
runtime.

This is an independent, clean-room implementation — see [`NOTICE`](NOTICE)
for details on what that means and how this project relates to other
on-screen keyboard projects.

## Features

- **Dual UI**: `KeyboardWidget` (QWidget) and `KeyboardPanel.qml` (Qt Quick)
  both render the same `KeyboardController`/`KeyboardTheme` C++ objects.
- **JSON-driven layouts**: every key is a self-describing object (`type`,
  `text`, `icon`, `target`, `span`); see [Layout format](#layout-format).
- **Theming**: colors, font, corner radius, and spacing are `Q_PROPERTY`s on
  a single `KeyboardTheme` object; change one at runtime and every key
  re-renders.
- **i18n**: per-locale key layouts (`layouts/en.json`, `layouts/ko.json`,
  ...) plus Qt Linguist translation of control-key labels (Enter, Backspace,
  Shift, ...).

## Status

The core library, both views, and the `en`/`ko` layouts are implemented.
The CMake build, example apps, and `.ts` translation files are tracked as
open work in the repository's issues — see those before assuming this
builds out of the box. Requires Qt 6.2+ (uses `QML_ELEMENT` and
`qt_add_qml_module`-style registration).

## Using the QWidget view

```cpp
#include "qkeyboardwidget/keyboard_widget.h"

auto *keyboard = new qkw::KeyboardWidget(parentWidget);
keyboard->controller()->loadFile(":/layouts/en.json"); // or an absolute path

// Wire it up to whatever text field you're typing into.
connect(keyboard->controller(), &qkw::KeyboardController::characterEntered,
        this, [this](const QString &text) { lineEdit->insert(text); });
connect(keyboard->controller(), &qkw::KeyboardController::backspaceRequested,
        this, [this]() { lineEdit->backspace(); });
connect(keyboard->controller(), &qkw::KeyboardController::enterRequested,
        this, [this]() { submit(); });

// Optional: re-skin at runtime.
keyboard->theme()->setKeyColor(QColor("#2b2b2e"));
keyboard->theme()->setAccentKeyColor(QColor("#ff9500"));
keyboard->theme()->setCornerRadius(10);
```

## Using the QML view

```qml
import QtQuick
import QKeyboardWidget

Item {
    KeyboardController {
        id: controller
        source: "qrc:/layouts/en.json"

        onCharacterEntered: (text) => inputField.insert(inputField.cursorPosition, text)
        onBackspaceRequested: inputField.remove(inputField.cursorPosition - 1, inputField.cursorPosition)
        onEnterRequested: submit()
    }

    KeyboardTheme {
        id: theme
        keyColor: "#2b2b2e"
        accentKeyColor: "#ff9500"
        cornerRadius: 10
    }

    KeyboardPanel {
        anchors.bottom: parent.bottom
        width: parent.width
        controller: controller
        theme: theme
    }
}
```

Requires linking `QtQuick`/`QtQuickLayouts` and building the QML module
(module URI `QKeyboardWidget`) so the `import QKeyboardWidget` resolves —
see the CMake build issue for current status.

## Switching languages at runtime

Two independent things can change per language:

1. **Key layout** — call `controller->loadFile(...)` (C++) or set
   `controller.source` (QML) to a different `layouts/<locale>.json`.
2. **UI string translation** (Enter/Backspace/Shift/... labels) — install a
   `QTranslator` loaded from `i18n/qkeyboardwidget_<locale>.qm` before
   reloading the layout, so `KeyboardController::resolveLabel()` picks up
   the translated strings on the next `rows` rebuild.

```cpp
auto *translator = new QTranslator(qApp);
translator->load(QLocale(QLocale::Korean), QStringLiteral("qkeyboardwidget"),
                  QStringLiteral("_"), QStringLiteral(":/i18n"));
qApp->installTranslator(translator);
keyboard->controller()->loadFile(":/layouts/ko.json");
```

## Layout format

```jsonc
{
  "locale": "en",
  "pages": [
    {
      "id": "lower",
      "rows": [
        [
          { "type": "char", "text": "q" },
          { "type": "backspace", "span": 2, "icon": ":/qkeyboardwidget/icons/backspace.svg" }
        ],
        [
          { "type": "shift", "target": "upper", "span": 2, "icon": ":/qkeyboardwidget/icons/shift.svg" },
          { "type": "switch", "target": "numeric", "labelId": "numbers", "span": 2 },
          { "type": "space", "span": 5 },
          { "type": "enter", "span": 2, "icon": ":/qkeyboardwidget/icons/enter.svg" }
        ]
      ]
    },
    { "id": "upper", "rows": [ "..." ] }
  ]
}
```

| Field | Meaning |
|---|---|
| `type` | `char`, `backspace`, `enter`, `space`, `shift`, or `switch`. |
| `text` | Literal characters inserted for `char` keys. |
| `labelId` | Translatable id for control-key display text (`enter`, `backspace`, `space`, `shift`, `numbers`, `letters`, `symbols`); falls back to a sane default per action if omitted. |
| `icon` | Resource/file path for the key's icon (optional). |
| `target` | Page `id` to jump to, for `shift`/`switch` keys. |
| `span` | Grid column span (default 1). |

Adding a new language is just adding a new `layouts/<locale>.json` file —
no code changes required. See `layouts/en.json` and `layouts/ko.json` for
complete examples.

> **Note on `layouts/ko.json`**: it emits individual Hangul jamo characters
> (ㅂ, ㅏ, ...). Composing them into syllable blocks (e.g. ㄱ+ㅏ+ㄴ → 간) is
> left to the receiving text field / OS IME; this library does not
> implement Hangul syllable composition itself (tracked as a known
> limitation in the issues).

## Repository layout

```
include/qkeyboardwidget/   public headers (namespace qkw)
src/                       implementation
qml/                       Qt Quick components
layouts/                   per-locale layout JSON
assets/icons/              key icons (SVG)
i18n/                      Qt Linguist translation sources (in progress)
examples/                  demo apps (in progress)
```

## License

Apache-2.0 — see [`LICENSE`](LICENSE) and [`NOTICE`](NOTICE).
