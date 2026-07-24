#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

#include "qkeyboard/qml_registration.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qkw::registerQmlTypes();

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral(QKW_QML_IMPORT_PATH));
    engine.load(QUrl::fromLocalFile(QStringLiteral(QKW_EXAMPLE_MAIN_QML)));

    if (engine.rootObjects().isEmpty()) return -1;

    return app.exec();
}
