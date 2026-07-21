#include <QtTest>

#include "qkeyboardwidget/keyboard_layout.h"

using namespace qkw;

class TestKeyboardLayout : public QObject
{
    Q_OBJECT

private slots:
    // Parsing — happy path
    void parsesValidLayout();
    void parsesAllKeyTypes();
    void parsesSpanField();
    void parsesMultiplePages();

    // Parsing — error cases
    void reportsErrorOnMalformedJson();
    void reportsErrorOnNonObjectRoot();
    void reportsErrorOnEmptyPages();
    void reportsErrorOnUnknownKeyType();
    void reportsErrorOnCharKeyMissingText();
    void reportsErrorOnShiftKeyMissingTarget();
    void reportsErrorOnSwitchKeyMissingLabelId();
    void reportsErrorOnPageMissingId();

    // Navigation
    void findsPageIndexById();
    void returnsMinusOneForMissingPage();

    // Default-constructed layout
    void defaultConstructedLayoutIsInvalid();
};

// ---------------------------------------------------------------------------
// Happy path
// ---------------------------------------------------------------------------

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

void TestKeyboardLayout::parsesAllKeyTypes()
{
    // Verify every recognised action type is parsed without error.
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "p1", "rows": [ [
                { "type": "char",      "text": "x" },
                { "type": "backspace" },
                { "type": "enter" },
                { "type": "space" },
                { "type": "shift",  "target": "p2" },
                { "type": "switch", "target": "p2", "labelId": "numbers" }
            ] ] },
            { "id": "p2", "rows": [ [ { "type": "char", "text": "y" } ] ] }
        ]
    })";

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY2(layout.isValid(), qPrintable(error));

    const auto &row = layout.pages().first().rows.first();
    QCOMPARE(static_cast<int>(row.at(0).action), static_cast<int>(KeyAction::Character));
    QCOMPARE(static_cast<int>(row.at(1).action), static_cast<int>(KeyAction::Backspace));
    QCOMPARE(static_cast<int>(row.at(2).action), static_cast<int>(KeyAction::Enter));
    QCOMPARE(static_cast<int>(row.at(3).action), static_cast<int>(KeyAction::Space));
    QCOMPARE(static_cast<int>(row.at(4).action), static_cast<int>(KeyAction::Shift));
    QCOMPARE(static_cast<int>(row.at(5).action), static_cast<int>(KeyAction::Switch));
}

void TestKeyboardLayout::parsesSpanField()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "id": "p", "rows": [ [
            { "type": "char", "text": "a", "span": 3 },
            { "type": "enter" }
        ] ] } ]
    })";

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY2(layout.isValid(), qPrintable(error));

    const auto &row = layout.pages().first().rows.first();
    QCOMPARE(row.at(0).span, 3);
    QCOMPARE(row.at(1).span, 1); // default span
}

void TestKeyboardLayout::parsesMultiplePages()
{
    const QByteArray json = R"({
        "locale": "ko",
        "pages": [
            { "id": "jamo",       "rows": [] },
            { "id": "jamo_shift", "rows": [] },
            { "id": "numeric",    "rows": [] },
            { "id": "symbols",    "rows": [] }
        ]
    })";

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("ko"));
    QCOMPARE(layout.pages().size(), 4);
    QCOMPARE(layout.pages().at(2).id, QStringLiteral("numeric"));
}

// ---------------------------------------------------------------------------
// Error cases
// ---------------------------------------------------------------------------

void TestKeyboardLayout::reportsErrorOnMalformedJson()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(QByteArrayLiteral("{ not json"), &error);
    QVERIFY(!layout.isValid());
    QVERIFY(!error.isEmpty());
}

void TestKeyboardLayout::reportsErrorOnNonObjectRoot()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(QByteArrayLiteral("[1,2,3]"), &error);
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

void TestKeyboardLayout::reportsErrorOnCharKeyMissingText()
{
    // A "char" key with no "text" field is invalid.
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "id": "lower", "rows": [ [ { "type": "char" } ] ] } ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(!error.isEmpty());
}

void TestKeyboardLayout::reportsErrorOnShiftKeyMissingTarget()
{
    // A "shift" key without a "target" page id is invalid.
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "id": "lower", "rows": [ [ { "type": "shift" } ] ] } ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(!error.isEmpty());
}

void TestKeyboardLayout::reportsErrorOnSwitchKeyMissingLabelId()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "lower", "rows": [
                [ { "type": "switch", "target": "numeric" } ]
            ] }
        ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("labelId")));
}

void TestKeyboardLayout::reportsErrorOnPageMissingId()
{
    // A page object with no "id" key is invalid.
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "rows": [] } ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(!error.isEmpty());
}

// ---------------------------------------------------------------------------
// Navigation helpers
// ---------------------------------------------------------------------------

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
    QCOMPARE(layout.indexOfPage(QStringLiteral("lower")), 0);
    QCOMPARE(layout.indexOfPage(QStringLiteral("upper")), 1);
    QCOMPARE(layout.indexOfPage(QStringLiteral("missing")), -1);
}

void TestKeyboardLayout::returnsMinusOneForMissingPage()
{
    const QByteArray json = R"({"locale":"en","pages":[{"id":"p","rows":[]}]})";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.indexOfPage(QStringLiteral("nonexistent")), -1);
}

// ---------------------------------------------------------------------------
// Default construction
// ---------------------------------------------------------------------------

void TestKeyboardLayout::defaultConstructedLayoutIsInvalid()
{
    const KeyboardLayout layout;
    QVERIFY(!layout.isValid());
    QVERIFY(layout.locale().isEmpty());
    QVERIFY(layout.pages().isEmpty());
    QCOMPARE(layout.indexOfPage(QStringLiteral("any")), -1);
}

QTEST_GUILESS_MAIN(TestKeyboardLayout)
#include "tst_keyboardlayout.moc"
