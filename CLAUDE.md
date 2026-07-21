# Project guidelines for QKeyboard

This file records the standing goals and conventions agreed for this
repository, so they carry forward across sessions instead of needing to be
re-stated.

## Project goals

1. **Freely usable regardless of license.** The project is Apache-2.0.
   Don't copy code, structure, or bundled assets from the original GPLv3
   `QKeyboardWidget` project (`Mihaylov93/QKeyboardWidget`) — this is an
   independent, clean-room reimplementation; only the general product
   concept (a JSON-driven on-screen keyboard widget for Qt) was carried
   over. See `NOTICE`.
2. **The QWidget and QML views must behave identically.** Both
   `KeyboardWidget` and `KeyboardPanel.qml` render the same
   `KeyboardController`/`KeyboardTheme` C++ core and nothing else. Never
   add layout-parsing or typing/paging logic to either view — that always
   belongs in `KeyboardController`/`KeyboardLayout`, so the two UIs can't
   drift apart in behavior.
3. **Must build against Qt5 (broad 5.x) through the latest Qt6.** Avoid
   APIs that only exist on Qt6 or Qt 5.15-backports (e.g. `QML_ELEMENT`,
   `qt_add_qml_module`, unversioned QML imports). Prefer version-agnostic
   patterns instead:
   - CMake: `find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)` then
     `find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ...)`.
   - QML type registration: imperative `qmlRegisterType<T>(...)` (see
     `qml_registration.h`/`.cpp`), not `QML_ELEMENT`.
   - QML files: versioned imports (e.g. `import QtQuick 2.0`), not
     unversioned Qt6-only imports.
   - Translations: `QCoreApplication::translate("QKeyboardWidget", ...)`
     with a fixed literal context, not `tr()`'s implicit class-name
     context, since that depends on how a given Qt/lupdate version mangles
     a namespaced QObject's class name.
4. **Follow Effective C++ (Scott Meyers) principles in C++ code.** In
   particular: const-correctness on getters and parameters that aren't
   mutated; pass non-trivial types by `const &`; explicit constructors to
   avoid implicit conversions; scoped enums (`enum class`) instead of
   plain `enum`; member-initializer lists; let Qt's QObject parent/child
   ownership do RAII instead of manual `new`/`delete` bookkeeping; only
   give a class a virtual destructor if it's actually used polymorphically
   as a base class.

## Workflow

- One branch per issue/feature (`feature/<name>` or `docs/<name>`), opened
  as a PR against `main`. Don't push directly to `main`.
- The repo owner reviews and merges every PR themselves — don't merge your
  own PRs.
- Track work as GitHub issues; reference them in commit messages/PR bodies
  (e.g. `Closes #N`).

## Engineering discipline

These exist because each was violated in practice — see the issues opened
2026-07-21 from a hostile self-review (#44-#55) for the concrete incidents.
Treat them as checks to run before opening or merging a PR, not aspirations.

1. **A change to one view's rendering/interaction logic needs the matching
   change in the other view, in the same PR.** Geometry, sizing (icon size,
   font auto-fit), accent/theme handling, and press feedback must match
   between `KeyboardWidget`/`KeyButton` (QWidget) and
   `KeyboardPanel.qml`/`KeyboardKey.qml` (QML) — this is project goal #2,
   applied concretely. State in the PR description how parity was checked
   (both example apps run side by side against the same layout/theme, or an
   automated test). A PR that touches key rendering in only one view is a
   sign something was missed, not a sign the other view didn't need it.
2. **Never re-derive enum-backed logic from raw literals in more than one
   place.** If QML needs to branch on a C++ enum (e.g. `KeyAction`), expose
   it as a named field from the core (`KeyDefinition::toVariantMap()`,
   `Q_ENUM`) — don't hardcode the same magic-number comparison
   independently in `KeyboardWidget` and `KeyboardKey.qml`.
3. **Parsers validate everything they accept, not just presence.**
   `KeyboardLayout`'s stated design goal is reporting malformed input as an
   error rather than defaulting around it silently. A new JSON field needs
   both an existence check and a validity check (numeric ranges,
   cross-references to other parsed data such as a `target` page id
   actually existing) before `parseKey()`/`fromJson()` accept it.
4. **Remove a workaround in the same change that supersedes it.** If a fix
   in `CMakeLists.txt`, CI config, or app code turns out not to work (or a
   different fix replaces it), delete the ineffective one immediately —
   don't leave inert code that looks load-bearing for the next reader.
   Relatedly: if the same problem could plausibly be fixed in two places
   (e.g. `CMakeLists.txt` vs. `.github/workflows/ci.yml`), fix it in exactly
   one — prefer CI-level for environment-specific issues (toolchain/SDK
   quirks), `CMakeLists.txt` for anything a downstream consumer also needs.
5. **One branch stays scoped to one concern.** A branch/PR that's already
   merged is done — per the existing "one branch per issue/feature" rule
   below, a new fix, feature, or unrelated CI change gets a new branch, even
   if it's tempting to keep pushing to a branch that's still checked out.
6. **A bug fix ships with a regression test in the same PR.** This
   project's actual shipped bugs (#24's nested-`Repeater` `modelData`
   shadowing, #25's `QStackedWidget` double-delete) were caught by manual
   review, not tests. A fix to `KeyboardWidget`, `KeyButton`, or a QML view
   without an accompanying QtTest/Qt Quick Test regression test is
   incomplete — and coverage numbers are only meaningful if they include
   the view code, not just the framework-agnostic core.

## Versioning

Semantic Versioning; current version is `0.1.0` (pre-1.0, API not yet
stable). The version lives in exactly one place —
`project(QKeyboardWidget VERSION X.Y.Z ...)` in the top-level
`CMakeLists.txt` — and the rules for what bumps MAJOR/MINOR/PATCH, and the
release checklist, are in `VERSIONING.md`. See that file before cutting a
release or deciding whether a change is breaking.
