#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

#include "qkeyboardwidget/qml_registration.h"

int main(int argc, char *argv[])
{
    // See the note in main.cpp on qkeyboardwidget/keyboard_widget.h's
    // widgets counterpart: layouts/icons are compiled into the static
    // library's own resources and need this to actually be linked in.
    Q_INIT_RESOURCE(qkeyboardwidget);

    QGuiApplication app(argc, argv);

    qkw::registerQmlTypes();

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral(QKW_QML_IMPORT_PATH));
    engine.load(QUrl::fromLocalFile(QStringLiteral(QKW_EXAMPLE_MAIN_QML)));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
