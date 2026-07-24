#include <QFile>
#include <QtTest>

#include "qkeyboardwidget/keyboard_layout.h"

using namespace qkw;

class TestLayouts : public QObject
{
    Q_OBJECT

private slots:
    void loadsEnglishResourceLayout();
    void loadsKoreanResourceLayout();
    void loadsSpanishResourceLayout();
    void loadsGermanResourceLayout();
    void loadsFrenchResourceLayout();
    void loadsRussianResourceLayout();
    void loadsJapaneseRomajiResourceLayout();
    void loadsJapaneseKanaResourceLayout();
};

void TestLayouts::loadsEnglishResourceLayout()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/en.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("en"));
}

void TestLayouts::loadsKoreanResourceLayout()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/ko.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("ko"));
}

void TestLayouts::loadsSpanishResourceLayout()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/es.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("es"));
}

void TestLayouts::loadsGermanResourceLayout()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/de.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("de"));
}

void TestLayouts::loadsFrenchResourceLayout()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/fr.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("fr"));
}

void TestLayouts::loadsRussianResourceLayout()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/ru.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("ru"));
}

void TestLayouts::loadsJapaneseRomajiResourceLayout()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/ja_romaji.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("ja"));
}

void TestLayouts::loadsJapaneseKanaResourceLayout()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/ja_kana.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("ja"));
}

QTEST_GUILESS_MAIN(TestLayouts)
#include "tst_layouts.moc"
