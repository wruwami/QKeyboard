#include <QQmlEngine>
#include <QtQuickTest/quicktest.h>

#include "qkeyboardwidget/qml_registration.h"

class Setup : public QObject
{
    Q_OBJECT
public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        qkw::registerQmlTypes();
        engine->addImportPath(QStringLiteral(QKW_QML_IMPORT_PATH));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(qkw_qml_tests, Setup)
#include "tst_quick.moc"
