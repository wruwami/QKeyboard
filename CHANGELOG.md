# Changelog

All notable changes to this project are documented here. The format is
based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this
project adheres to the versioning rules in [`VERSIONING.md`](VERSIONING.md).

## [Unreleased]

## [0.1.0] - 2026-07-21

Initial release.

### Added

- Framework-agnostic core (`KeyDefinition`, `KeyboardLayout`,
  `KeyboardController`): JSON layout parsing with error reporting, page
  switching, and `characterEntered`/`backspaceRequested`/`enterRequested`
  signals.
- `KeyboardWidget`/`KeyButton`: QWidget rendering of the core.
- `KeyboardPanel.qml`/`KeyboardKey.qml`: Qt Quick rendering of the same
  core, registered via imperative `qmlRegisterType()`.
- `KeyboardTheme`: live-reskinnable colors, font, corner radius, and
  spacing shared by both views.
- `layouts/en.json` and `layouts/ko.json` locale layouts.
- `qkw::HangulComposer`: opt-in Hangul jamo-to-syllable composition helper.
- UI-string translation scaffolding (`i18n/*.ts`, Qt Linguist).
- CMake build supporting Qt5 (broad 5.x) through the latest Qt6, with an
  optional QML module and translation compilation.
- `examples/widgets_example` and `examples/qml_example` demo apps.
- CI (build + QtTest + coverage) and README badges.

[Unreleased]: https://github.com/wruwami/QKeyboard/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/wruwami/QKeyboard/releases/tag/v0.1.0
