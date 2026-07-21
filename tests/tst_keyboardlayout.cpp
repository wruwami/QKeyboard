#include <QtTest>

#include "qkeyboardwidget/keyboard_layout.h"

using namespace qkw;

class TestKeyboardLayout : public QObject
{
    Q_OBJECT

private slots:
    void parsesValidLayout();
    void reportsErrorOnMalformedJson();
    void reportsErrorOnEmptyPages();
    void reportsErrorOnUnknownKeyType();
    void findsPageIndexById();
};

void TestKeyboardLayout::parsesValidLayout()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "lower", "rows": [
                [ { "type": "char", "text": "a" }, { "type": "backspace", "span": 2 } ]
            ] }
        ]
    })";

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);

    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("en"));
    QCOMPARE(layout.pages().size(), 1);
    QCOMPARE(layout.pages().first().id, QStringLiteral("lower"));
    QCOMPARE(layout.pages().first().rows.size(), 1);
    QCOMPARE(layout.pages().first().rows.first().size(), 2);

    const KeyDefinition &charKey = layout.pages().first().rows.first().at(0);
    QCOMPARE(static_cast<int>(charKey.action), static_cast<int>(KeyAction::Character));
    QCOMPARE(charKey.text, QStringLiteral("a"));

    const KeyDefinition &backspaceKey = layout.pages().first().rows.first().at(1);
    QCOMPARE(static_cast<int>(backspaceKey.action), static_cast<int>(KeyAction::Backspace));
    QCOMPARE(backspaceKey.span, 2);
}

void TestKeyboardLayout::reportsErrorOnMalformedJson()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(QByteArrayLiteral("{ not json"), &error);
    QVERIFY(!layout.isValid());
    QVERIFY(!error.isEmpty());
}

void TestKeyboardLayout::reportsErrorOnEmptyPages()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(R"({"locale":"en","pages":[]})", &error);
    QVERIFY(!layout.isValid());
    QVERIFY(!error.isEmpty());
}

void TestKeyboardLayout::reportsErrorOnUnknownKeyType()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "id": "lower", "rows": [ [ { "type": "bogus" } ] ] } ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("bogus")));
}

void TestKeyboardLayout::findsPageIndexById()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "lower", "rows": [] },
            { "id": "upper", "rows": [] }
        ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.indexOfPage(QStringLiteral("upper")), 1);
    QCOMPARE(layout.indexOfPage(QStringLiteral("missing")), -1);
}

QTEST_GUILESS_MAIN(TestKeyboardLayout)
#include "tst_keyboardlayout.moc"
