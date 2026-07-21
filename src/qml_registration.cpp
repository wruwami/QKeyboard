#include "qkeyboardwidget/qml_registration.h"

#ifdef QKW_ENABLE_QML

#include <QtQml/qqml.h>

#include "qkeyboardwidget/keyboard_controller.h"
#include "qkeyboardwidget/keyboard_theme.h"

namespace qkw {

void registerQmlTypes(const char *uri, int versionMajor, int versionMinor)
{
    qmlRegisterType<KeyboardController>(uri, versionMajor, versionMinor, "KeyboardController");
    qmlRegisterType<KeyboardTheme>(uri, versionMajor, versionMinor, "KeyboardTheme");
}

} // namespace qkw

#endif // QKW_ENABLE_QML
