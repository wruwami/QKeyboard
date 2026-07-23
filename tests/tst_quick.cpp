#include <QQmlEngine>
#include <QtQuickTest/quicktest.h>

#include "qkeyboardwidget/qml_registration.h"

class Setup : public QObject
{
    Q_OBJECT
public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        // Needed for tst_KeyboardPanel.qml's icon tests to find the
        // library's compiled-in ":/qkeyboardwidget/icons/*.svg" resources:
        // statically linking qkeyboardwidget alone doesn't run its .qrc's
        // resource-initializer, since nothing else in this binary
        // references those symbols directly (same reason the example apps
        // and other tests call this too).
        Q_INIT_RESOURCE(qkeyboardwidget);
        qkw::registerQmlTypes();
        engine->addImportPath(QStringLiteral(QKW_QML_IMPORT_PATH));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(qkw_qml_tests, Setup)
#include "tst_quick.moc"
