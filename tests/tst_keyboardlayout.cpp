#include <QTemporaryFile>
#include <QtTest>
#include <QBuffer>
#include <QStringList>

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
    void reportsErrorOnNonArrayRow();
    void reportsErrorOnZeroSpan();
    void reportsErrorOnNegativeSpan();
    void reportsErrorOnShiftTargetNotAKnownPage();
    void reportsErrorOnSwitchTargetNotAKnownPage();

    // File loading
    void fromFileReportsErrorForMissingFile();
    void fromFileLoadsAValidFile();

    // Navigation
    void findsPageIndexById();
    void returnsMinusOneForMissingPage();

    // Default-constructed layout
    void defaultConstructedLayoutIsInvalid();

    // New tests for issue #82
    void parsesLayoutFromDevice();
    void parsesLayoutFromFile();
    void reportsErrorOnRowNotJsonArray();
    void reportsErrorOnKeyNotJsonObject();

    // New tests for issue #92 (JSON Schema & validation)
    void reportsErrorOnMissingOrEmptyLocale();
    void reportsErrorOnNonArrayPages();
    void validatesSchemaAndLoadsAllProjectLayouts();
    void reportsErrorOnNonStringLocale();
    void reportsErrorOnPagesAsNonArrayJsonValue_data();
    void reportsErrorOnPagesAsNonArrayJsonValue();
    void acceptsWhitespaceOnlyLocaleAsPresentButNonEmpty();
    void localeErrorTakesPrecedenceOverPagesError();
    void schemaTopLevelRequiresLocaleAndPagesAndForbidsExtras();
    void schemaKeyDefinitionEnumMatchesSupportedKeyTypes();
    void schemaConditionalRequirementsForShiftAndSwitchKeys();
    void qrcResourceRegistersSchemaFileAlongsideLayouts();
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

void TestKeyboardLayout::reportsErrorOnNonArrayRow()
{
    // A "rows" entry that isn't itself a JSON array (each row must be one).
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "id": "lower", "rows": [ "not-a-row" ] } ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(!error.isEmpty());
}

void TestKeyboardLayout::reportsErrorOnZeroSpan()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "id": "lower", "rows": [ [ { "type": "char", "text": "a", "span": 0 } ] ] } ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("span")));
}

void TestKeyboardLayout::reportsErrorOnNegativeSpan()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "id": "lower", "rows": [ [ { "type": "char", "text": "a", "span": -3 } ] ] } ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("span")));
}

void TestKeyboardLayout::reportsErrorOnShiftTargetNotAKnownPage()
{
    // "target" is non-empty (so the earlier presence check passes) but
    // doesn't match any page id actually declared in this layout.
    const QByteArray json = R"({
        "locale": "en",
        "pages": [ { "id": "lower", "rows": [ [ { "type": "shift", "target": "numeriic" } ] ] } ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("numeriic")));
}

void TestKeyboardLayout::reportsErrorOnSwitchTargetNotAKnownPage()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "lower", "rows": [
                [ { "type": "switch", "target": "nonexistent", "labelId": "numbers" } ]
            ] }
        ]
    })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("nonexistent")));
}

// ---------------------------------------------------------------------------
// File loading
// ---------------------------------------------------------------------------

void TestKeyboardLayout::fromFileReportsErrorForMissingFile()
{
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral("/nonexistent/path.json"), &error);
    QVERIFY(!layout.isValid());
    QVERIFY(!error.isEmpty());
}

void TestKeyboardLayout::fromFileLoadsAValidFile()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write(R"({"locale":"en","pages":[{"id":"p","rows":[]}]})");
    file.close();

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(file.fileName(), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("en"));
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

void TestKeyboardLayout::parsesLayoutFromDevice()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "p", "rows": [ [ { "type": "char", "text": "a" } ] ] }
        ]
    })";
    QBuffer buffer;
    buffer.setData(json);
    buffer.open(QIODevice::ReadOnly);

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromDevice(buffer, &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("en"));
}

void TestKeyboardLayout::parsesLayoutFromFile()
{
    Q_INIT_RESOURCE(qkeyboardwidget);

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/en.json"), &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("en"));
}

void TestKeyboardLayout::reportsErrorOnRowNotJsonArray()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "p", "rows": [ {} ] }
        ]
    })";

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("JSON array")));
}

void TestKeyboardLayout::reportsErrorOnKeyNotJsonObject()
{
    const QByteArray json = R"({
        "locale": "en",
        "pages": [
            { "id": "p", "rows": [ [ 42 ] ] }
        ]
    })";

    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("JSON object")));
}

void TestKeyboardLayout::reportsErrorOnMissingOrEmptyLocale()
{
    {
        const QByteArray json = R"({ "pages": [ { "id": "p", "rows": [] } ] })";
        QString error;
        const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
        QVERIFY(!layout.isValid());
        QVERIFY(error.contains(QStringLiteral("locale")));
    }
    {
        const QByteArray json = R"({ "locale": "", "pages": [ { "id": "p", "rows": [] } ] })";
        QString error;
        const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
        QVERIFY(!layout.isValid());
        QVERIFY(error.contains(QStringLiteral("locale")));
    }
}

void TestKeyboardLayout::reportsErrorOnNonArrayPages()
{
    const QByteArray json = R"({ "locale": "en", "pages": "not-an-array" })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("pages")));
}

void TestKeyboardLayout::validatesSchemaAndLoadsAllProjectLayouts()
{
    Q_INIT_RESOURCE(qkeyboardwidget);

    // Validate schema file resource is valid JSON document
    QFile schemaFile(QStringLiteral(":/layouts/schema/keyboard-layout.schema.json"));
    QVERIFY(schemaFile.open(QIODevice::ReadOnly));
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(schemaFile.readAll(), &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object().value(QStringLiteral("title")).toString(), QStringLiteral("QKeyboard Layout Schema"));

    // Validate project layout files en.json and ko.json load cleanly
    QString error;
    const KeyboardLayout enLayout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/en.json"), &error);
    QVERIFY2(enLayout.isValid(), qPrintable(error));
    QCOMPARE(enLayout.locale(), QStringLiteral("en"));

    const KeyboardLayout koLayout = KeyboardLayout::fromFile(QStringLiteral(":/layouts/ko.json"), &error);
    QVERIFY2(koLayout.isValid(), qPrintable(error));
    QCOMPARE(koLayout.locale(), QStringLiteral("ko"));
}

void TestKeyboardLayout::reportsErrorOnNonStringLocale()
{
    // "locale" present but not a JSON string (e.g. a number) must be
    // rejected the same way a missing/empty locale is.
    const QByteArray json = R"({ "locale": 42, "pages": [ { "id": "p", "rows": [] } ] })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("locale")));
}

void TestKeyboardLayout::reportsErrorOnPagesAsNonArrayJsonValue_data()
{
    QTest::addColumn<QByteArray>("json");

    QTest::newRow("pages is an object") << QByteArray(
        R"({ "locale": "en", "pages": { "id": "p" } })");
    QTest::newRow("pages is null") << QByteArray(
        R"({ "locale": "en", "pages": null })");
    QTest::newRow("pages is a number") << QByteArray(
        R"({ "locale": "en", "pages": 7 })");
    QTest::newRow("pages is a bool") << QByteArray(
        R"({ "locale": "en", "pages": true })");
}

void TestKeyboardLayout::reportsErrorOnPagesAsNonArrayJsonValue()
{
    QFETCH(QByteArray, json);
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("pages")));
}

void TestKeyboardLayout::acceptsWhitespaceOnlyLocaleAsPresentButNonEmpty()
{
    // The locale guard only rejects an empty string; a whitespace-only
    // value is a non-empty QString and therefore currently passes. This
    // pins down that boundary so a future change to the emptiness check
    // (e.g. trimming) is a deliberate, visible behavior change.
    const QByteArray json = R"({ "locale": "   ", "pages": [ { "id": "p", "rows": [] } ] })";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY2(layout.isValid(), qPrintable(error));
    QCOMPARE(layout.locale(), QStringLiteral("   "));
}

void TestKeyboardLayout::localeErrorTakesPrecedenceOverPagesError()
{
    // When both 'locale' and 'pages' are missing, the locale check runs
    // first, so the reported error must be about 'locale', not 'pages'.
    const QByteArray json = R"({})";
    QString error;
    const KeyboardLayout layout = KeyboardLayout::fromJson(json, &error);
    QVERIFY(!layout.isValid());
    QVERIFY(error.contains(QStringLiteral("locale")));
}

void TestKeyboardLayout::schemaTopLevelRequiresLocaleAndPagesAndForbidsExtras()
{
    Q_INIT_RESOURCE(qkeyboardwidget);

    QFile schemaFile(QStringLiteral(":/layouts/schema/keyboard-layout.schema.json"));
    QVERIFY(schemaFile.open(QIODevice::ReadOnly));
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(schemaFile.readAll(), &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    const QJsonObject root = doc.object();

    QCOMPARE(root.value(QStringLiteral("type")).toString(), QStringLiteral("object"));
    QCOMPARE(root.value(QStringLiteral("additionalProperties")).toBool(true), false);

    const QJsonArray requiredArray = root.value(QStringLiteral("required")).toArray();
    QVERIFY(requiredArray.contains(QJsonValue(QStringLiteral("locale"))));
    QVERIFY(requiredArray.contains(QJsonValue(QStringLiteral("pages"))));
    QCOMPARE(requiredArray.size(), 2);

    const QJsonObject properties = root.value(QStringLiteral("properties")).toObject();
    const QJsonObject localeSchema = properties.value(QStringLiteral("locale")).toObject();
    QCOMPARE(localeSchema.value(QStringLiteral("type")).toString(), QStringLiteral("string"));
    QCOMPARE(localeSchema.value(QStringLiteral("minLength")).toInt(), 1);

    const QJsonObject pagesSchema = properties.value(QStringLiteral("pages")).toObject();
    QCOMPARE(pagesSchema.value(QStringLiteral("type")).toString(), QStringLiteral("array"));
    QCOMPARE(pagesSchema.value(QStringLiteral("minItems")).toInt(), 1);
}

void TestKeyboardLayout::schemaKeyDefinitionEnumMatchesSupportedKeyTypes()
{
    Q_INIT_RESOURCE(qkeyboardwidget);

    QFile schemaFile(QStringLiteral(":/layouts/schema/keyboard-layout.schema.json"));
    QVERIFY(schemaFile.open(QIODevice::ReadOnly));
    const QJsonDocument doc = QJsonDocument::fromJson(schemaFile.readAll());
    const QJsonObject definitions = doc.object().value(QStringLiteral("definitions")).toObject();
    const QJsonObject keyDefinition = definitions.value(QStringLiteral("keyDefinition")).toObject();
    const QJsonObject typeSchema = keyDefinition.value(QStringLiteral("properties")).toObject().value(QStringLiteral("type")).toObject();

    QCOMPARE(typeSchema.value(QStringLiteral("type")).toString(), QStringLiteral("string"));

    const QJsonArray enumArray = typeSchema.value(QStringLiteral("enum")).toArray();
    QStringList enumValues;
    for (const QJsonValue &value : enumArray) enumValues << value.toString();

    // Must exactly match the key types accepted by KeyboardLayout::fromJson's
    // internal actionFromString() table (src/keyboard_layout.cpp).
    const QStringList expected = {
        QStringLiteral("char"),      QStringLiteral("backspace"), QStringLiteral("enter"),
        QStringLiteral("space"),     QStringLiteral("shift"),     QStringLiteral("switch"),
    };
    QCOMPARE(enumValues.size(), expected.size());
    for (const QString &type : expected) QVERIFY(enumValues.contains(type));
}

void TestKeyboardLayout::schemaConditionalRequirementsForShiftAndSwitchKeys()
{
    Q_INIT_RESOURCE(qkeyboardwidget);

    QFile schemaFile(QStringLiteral(":/layouts/schema/keyboard-layout.schema.json"));
    QVERIFY(schemaFile.open(QIODevice::ReadOnly));
    const QJsonDocument doc = QJsonDocument::fromJson(schemaFile.readAll());
    const QJsonObject keyDefinition =
        doc.object().value(QStringLiteral("definitions")).toObject().value(QStringLiteral("keyDefinition")).toObject();

    const QJsonArray allOf = keyDefinition.value(QStringLiteral("allOf")).toArray();
    QCOMPARE(allOf.size(), 3);

    bool foundCharRule = false;
    bool foundShiftSwitchTargetRule = false;
    bool foundSwitchLabelIdRule = false;

    for (const QJsonValue &clauseValue : allOf) {
        const QJsonObject clause = clauseValue.toObject();
        const QJsonObject ifTypeObj =
            clause.value(QStringLiteral("if")).toObject().value(QStringLiteral("properties")).toObject().value(QStringLiteral("type")).toObject();
        const QJsonArray thenRequired = clause.value(QStringLiteral("then")).toObject().value(QStringLiteral("required")).toArray();

        if (ifTypeObj.value(QStringLiteral("const")).toString() == QStringLiteral("char")) {
            QVERIFY(thenRequired.contains(QJsonValue(QStringLiteral("text"))));
            foundCharRule = true;
        }
        if (ifTypeObj.contains(QStringLiteral("enum"))) {
            const QJsonArray typesEnum = ifTypeObj.value(QStringLiteral("enum")).toArray();
            QStringList types;
            for (const QJsonValue &t : typesEnum) types << t.toString();
            if (types.contains(QStringLiteral("shift")) && types.contains(QStringLiteral("switch"))) {
                QVERIFY(thenRequired.contains(QJsonValue(QStringLiteral("target"))));
                foundShiftSwitchTargetRule = true;
            }
        }
        if (ifTypeObj.value(QStringLiteral("const")).toString() == QStringLiteral("switch")) {
            QVERIFY(thenRequired.contains(QJsonValue(QStringLiteral("labelId"))));
            foundSwitchLabelIdRule = true;
        }
    }

    QVERIFY(foundCharRule);
    QVERIFY(foundShiftSwitchTargetRule);
    QVERIFY(foundSwitchLabelIdRule);
}

void TestKeyboardLayout::qrcResourceRegistersSchemaFileAlongsideLayouts()
{
    Q_INIT_RESOURCE(qkeyboardwidget);

    // The qrc change adds the schema file under the existing /layouts
    // prefix without disturbing the existing en.json / ko.json entries.
    QVERIFY(QFile::exists(QStringLiteral(":/layouts/en.json")));
    QVERIFY(QFile::exists(QStringLiteral(":/layouts/ko.json")));
    QVERIFY(QFile::exists(QStringLiteral(":/layouts/schema/keyboard-layout.schema.json")));

    QFile schemaFile(QStringLiteral(":/layouts/schema/keyboard-layout.schema.json"));
    QVERIFY(schemaFile.open(QIODevice::ReadOnly));
    QVERIFY(schemaFile.size() > 0);
}

QTEST_GUILESS_MAIN(TestKeyboardLayout)
#include "tst_keyboardlayout.moc"
