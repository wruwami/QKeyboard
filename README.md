# QKeyboard

[![CI](https://github.com/wruwami/QKeyboard/actions/workflows/ci.yml/badge.svg)](https://github.com/wruwami/QKeyboard/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/wruwami/QKeyboard/branch/main/graph/badge.svg)](https://codecov.io/gh/wruwami/QKeyboard)

> **Notice:** This is a temporary personal project, developed with the
> assistance of AI. Feel free to try it out, but it's still early-stage
> and evolving quickly, so it's not really at the point of being a
> finished "product" yet.

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
- **Hangul composition**: `qkw::HangulComposer` is an opt-in helper that
  turns the jamo `layouts/ko.json` emits one at a time into precomposed
  Hangul syllable blocks; see the note under [Layout format](#layout-format).

## Status

The core library, both views, the CMake build, the `en`/`ko` layouts,
UI-string translation, and the example apps are implemented. Check the
repository's issues for anything still open before assuming this builds out
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
   `QTranslator` loaded from the compiled `qkeyboardwidget_<locale>.qm`
   before reloading the layout, so `KeyboardController::resolveLabel()`
   picks up the translated strings on the next `rows` rebuild. English needs
   no translator at all: `QCoreApplication::translate()` falls back to the
   (English) source text when no translator is installed for the active
   locale.

```cpp
auto *translator = new QTranslator(qApp);
// CMAKE_INSTALL_DATADIR/qkeyboardwidget/i18n if installed system-wide, or
// wherever your app embeds/deploys the .qm files it needs.
translator->load(QLocale(QLocale::Korean), QStringLiteral("qkeyboardwidget"),
                  QStringLiteral("_"), QStringLiteral("/usr/share/qkeyboardwidget/i18n"));
qApp->installTranslator(translator);
keyboard->controller()->loadFile(":/layouts/ko.json");
```

`i18n/qkeyboardwidget_ko.ts` is the Korean translation source; `QKW_BUILD_TRANSLATIONS`
(on by default) compiles it to `.qm` via CMake at build time and installs it.
To add another language, add `i18n/qkeyboardwidget_<locale>.ts` (run `lupdate`
against `src/keyboard_controller.cpp`, context `QKeyboardWidget`, to seed it
with the current source strings), list it in `QKW_TS_FILES` in
`CMakeLists.txt`, and translate it in Qt Linguist.

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
| `labelId` | Translatable id for control-key display text (`enter`, `backspace`, `space`, `shift`, `numbers`, `letters`, `symbols`); falls back to a sane default per action if omitted, except `switch` keys, which must specify it explicitly (there is no single default label for a page-switch key). |
| `icon` | Resource/file path for the key's icon (optional). |
| `target` | Page `id` to jump to, for `shift`/`switch` keys. |
| `span` | Grid column span (default 1). |

Adding a new language is just adding a new `layouts/<locale>.json` file —
no code changes required. See `layouts/en.json` and `layouts/ko.json` for
complete examples.

> **Note on `layouts/ko.json`**: it emits individual Hangul jamo characters
> (ㅂ, ㅏ, ...) one at a time, same as pressing each key on a physical
> 2-beolsik keyboard. Composing them into syllable blocks (e.g. ㄱ+ㅏ+ㄴ →
> 간) is an opt-in step: feed `KeyboardController::characterEntered` /
> `backspaceRequested` through a `qkw::HangulComposer` (see
> `include/qkeyboardwidget/hangul_composer.h` and
> `examples/widgets_example/main.cpp`) before touching your text field. Left
> unwired, jamo are inserted uncomposed and composition falls back to
> whatever the receiving text field / OS IME does with them.

## Examples

`examples/widgets_example` and `examples/qml_example` are both minimal demo
apps: a text field driven entirely by the keyboard, a combo box that swaps
the `en`/`ko` layout at runtime, and a small `KeyboardTheme` override. Build
them with the rest of the project (they're on by default) and run
`qkw_widgets_example` / `qkw_qml_example` from the build directory.

## Testing & CI

```sh
cmake -S . -B build -DQKW_ENABLE_COVERAGE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

`.github/workflows/ci.yml` runs this same build+test on a full matrix of
Linux, Windows, and macOS, each on both Qt 5.15 and Qt 6.7 (the two ends of
the "Qt5 through the latest Qt6" support range) — six jobs on every
push/PR. Coverage (lcov, GCC-only) is collected once, from the
Linux+Qt6 leg, and uploaded to Codecov. To get the Codecov badge actually
reporting data, the repo owner needs to connect `wruwami/QKeyboard` at
[codecov.io](https://codecov.io) and add the resulting upload token as a
`CODECOV_TOKEN` repository secret — that step can't be done from CI itself.

The Windows leg runs on a self-hosted runner (labels `self-hosted`,
`Windows`, `X64`) instead of GitHub-hosted `windows-latest`, since the
shared Actions minutes quota for this repo is exhausted; Linux and macOS
stay on GitHub-hosted runners. Switch it back to `windows-latest` in
`build-and-test`'s `runs-on` once quota is available again.

That self-hosted runner sits behind a corporate network that TLS-inspects
outbound HTTPS with its own root certificate, which breaks Python's
`requests`/`urllib3` (used by `aqtinstall` to download Qt) with a
self-signed-certificate verification error. The "Disable TLS verification
for Qt download" step works around this — scoped to the Windows leg only,
via a `sitecustomize.py` on `PYTHONPATH` rather than a machine-wide
change — per an explicit call by the repo owner that it's unavoidable on
this network. Remove that step (and the job-level `PYTHONPATH` env) if the
runner ever moves off this network or gets the corporate root CA properly
trusted instead.

A separate `lint` job runs on every push/PR too:

```sh
# formatting — must produce no diff
find src include tests examples -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror

# static analysis
cppcheck --std=c++17 --language=c++ --enable=warning,style,performance,portability \
  --inline-suppr --suppressions-list=.cppcheck-suppressions --error-exitcode=1 -I include \
  src include tests examples
```

Formatting rules live in [`.clang-format`](.clang-format) (matches the
style already used throughout the codebase — 4-space indent, brace
placement, pointer/reference alignment); run `clang-format -i <file>`
locally before committing. cppcheck's suppression list
([`.cppcheck-suppressions`](.cppcheck-suppressions)) is kept to genuine
false positives (e.g. Qt headers/moc output cppcheck can't resolve without
a full Qt install) — see the comments in that file before adding to it.

## Repository layout

```
include/qkeyboardwidget/   public headers (namespace qkw)
src/                       implementation
qml/                       Qt Quick components
layouts/                   per-locale layout JSON
assets/icons/              key icons (SVG)
i18n/                      Qt Linguist translation sources
examples/                  demo apps
```

## Versioning

This project follows [Semantic Versioning](https://semver.org/); see
[`VERSIONING.md`](VERSIONING.md) for the exact rules (the project is
currently pre-1.0, so the public API isn't stable yet) and
[`CHANGELOG.md`](CHANGELOG.md) for what changed in each release.

## License

Apache-2.0 — see [`LICENSE`](LICENSE) and [`NOTICE`](NOTICE).
