# Versioning

QKeyboard follows [Semantic Versioning 2.0.0](https://semver.org/):
`MAJOR.MINOR.PATCH`.

The single source of truth for the current version is
`project(QKeyboardWidget VERSION X.Y.Z ...)` in the top-level
`CMakeLists.txt`. Nothing else (README, tags, packaging metadata) should
hardcode a version separately from it.

## Current status: 0.1.0 (pre-1.0)

The project is pre-1.0: the public API (headers under
`include/qkeyboardwidget/`, the `Q_PROPERTY`/`Q_INVOKABLE` surface, the QML
types in `qml/`, and the `layouts/*.json` schema) is **not yet stable** and
may change in a MINOR bump. Per the SemVer spec, `0.x.y` gives no
compatibility guarantee — treat it accordingly until 1.0.0.

That said, within `0.x`, this project draws the line between MINOR and
PATCH more strictly than SemVer requires, so consumers still get useful
signal:

- **PATCH** (`0.1.0` → `0.1.1`): bug fixes only. No new public symbols, no
  behavior changes a caller could reasonably depend on.
- **MINOR** (`0.1.0` → `0.2.0`): backwards-compatible additions — a new
  class (e.g. `HangulComposer`), a new `Q_PROPERTY`, a new locale layout, a
  new optional parameter. May also include the breaking changes SemVer
  permits pre-1.0 (e.g. renaming a signal) when there isn't a
  non-breaking way to land the change — call this out prominently in the
  changelog entry when it happens.
- **MAJOR** stays at `0` until [1.0.0 criteria](#reaching-100) are met.

## After 1.0.0

Once the project reaches 1.0.0, standard SemVer compatibility rules apply
to the public API surface listed above:

- **MAJOR**: breaking change to any public header, `Q_PROPERTY`,
  `Q_INVOKABLE`/signal signature, QML type, or the `layouts/*.json` schema
  (a previously-valid layout file must keep parsing the same way).
- **MINOR**: backwards-compatible feature addition.
- **PATCH**: backwards-compatible bug fix only.

Internal-only files (`src/*.cpp`, private members, `examples/`, `tests/`)
are not part of the compatibility surface and can change in any release.

## Reaching 1.0.0

Bump to 1.0.0 once the core feature set is implemented and the public API
has had at least one release cycle without a breaking change. As a
checklist (see the repo's issue tracker for live status): core model,
QWidget view, QML view, theming, i18n scaffolding, and at least the initial
locale layouts should be implemented and stable before declaring 1.0.0 —
new locale layouts or optional composer helpers (e.g. future Japanese/
Chinese input support) do not need to land first and can ship as MINOR
releases afterward.

## Releasing

1. Update `project(... VERSION X.Y.Z ...)` in `CMakeLists.txt`.
2. Update `CHANGELOG.md` (add a dated section for the new version, moving
   entries out of `Unreleased`).
3. Commit as `chore(release): vX.Y.Z`.
4. Tag the commit `vX.Y.Z` (annotated tag) and push the tag.
5. Create a GitHub Release from the tag using the changelog section as the
   body.

Tags always have a `v` prefix (`v0.1.0`), matching the CMake version
without the prefix.
