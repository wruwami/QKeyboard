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

The core library, both views, the CMake build, and the `en`/`ko` layouts are
implemented. Example apps and `.ts` translation files are tracked as open
work in the repository's issues — see those before assuming this builds out
of the box.

Supports Qt5 (broad 5.x) through the latest Qt6: the QML types are
registered imperatively with `qmlRegisterType()` (see
[Using the QML view](#using-the-qml-view)) rather than the `QML_ELEMENT`
macro, since `QML_ELEMENT` needs Qt 5.15+/Qt6 and this library doesn't
assume that.

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

Register the C++ types once, before loading any QML that imports
`QKeyboardWidget`, and add `qml/` to the import path so the `qmldir` there
(which declares the `KeyboardKey`/`KeyboardPanel` QML components) is found:

```cpp
#include <QQmlApplicationEngine>
#include "qkeyboardwidget/qml_registration.h"

QQmlApplicationEngine engine;
qkw::registerQmlTypes(); // registers KeyboardController and KeyboardTheme
engine.addImportPath(QStringLiteral(":/qkeyboardwidget/qml")); // or wherever qml/ is deployed
engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
```

```qml
import QtQuick 2.0
import QKeyboardWidget 1.0

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

Requires linking `Qml`/`Quick` (`QKW_ENABLE_QML`, on by default in
`CMakeLists.txt`). `qml/KeyboardKey.qml` and `qml/KeyboardPanel.qml` are
shipped as plain files plus a hand-written `qml/qmldir` — not a compiled
Qt6-only `qt_add_qml_module` — so the same setup works on Qt5 and Qt6.

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
