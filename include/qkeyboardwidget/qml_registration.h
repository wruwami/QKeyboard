#pragma once

// Only declared when QML support is compiled in (see CMakeLists.txt's
// QKW_ENABLE_QML option, which also defines this macro for consumers).
#ifdef QKW_ENABLE_QML

#include "qkeyboardwidget/qkw_export.h"

namespace qkw {

// Registers KeyboardController and KeyboardTheme as QML types under the
// given module URI. Call this once, before loading any QML that uses them
// (typically right after constructing QGuiApplication/QApplication).
//
// Uses the imperative qmlRegisterType<T>() API instead of the QML_ELEMENT
// macro because QML_ELEMENT requires Qt 5.15+/Qt6, while this function works
// unchanged from Qt5.0 through the latest Qt6.
QKW_EXPORT void registerQmlTypes(const char *uri = "QKeyboardWidget", int versionMajor = 1, int versionMinor = 0);

} // namespace qkw

#endif // QKW_ENABLE_QML
