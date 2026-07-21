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

## Versioning

Semantic Versioning; current version is `0.1.0` (pre-1.0, API not yet
stable). The version lives in exactly one place —
`project(QKeyboardWidget VERSION X.Y.Z ...)` in the top-level
`CMakeLists.txt` — and the rules for what bumps MAJOR/MINOR/PATCH, and the
release checklist, are in `VERSIONING.md`. See that file before cutting a
release or deciding whether a change is breaking.
