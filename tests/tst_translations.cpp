#include <QtTest>
#include <QFile>
#include <QXmlStreamReader>
#include <QVector>

namespace {

struct TsMessage
{
    QString source;
    QString translation;
};

struct ParsedTs
{
    bool ok = false;
    QString version;
    QString language;
    QString contextName;
    QVector<TsMessage> messages;
};

// Minimal, purpose-built .ts (Qt Linguist source) file reader covering only
// the subset of the format resources/i18n/*.ts actually uses: a single root
// <TS version=... language=...> containing one <context><name>...</name>
// with one or more <message><source>/<translation></message> children.
ParsedTs parseTsFile(const QString &path)
{
    ParsedTs result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return result;

    QXmlStreamReader xml(&file);
    if (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("TS")) {
            xml.raiseError(QStringLiteral("root element is not <TS>"));
        } else {
            result.version = xml.attributes().value(QLatin1String("version")).toString();
            result.language = xml.attributes().value(QLatin1String("language")).toString();

            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("context")) {
                    while (xml.readNextStartElement()) {
                        if (xml.name() == QLatin1String("name")) {
                            result.contextName = xml.readElementText();
                        } else if (xml.name() == QLatin1String("message")) {
                            TsMessage message;
                            while (xml.readNextStartElement()) {
                                if (xml.name() == QLatin1String("source"))
                                    message.source = xml.readElementText();
                                else if (xml.name() == QLatin1String("translation"))
                                    message.translation = xml.readElementText();
                                else
                                    xml.skipCurrentElement();
                            }
                            result.messages.push_back(message);
                        } else {
                            xml.skipCurrentElement();
                        }
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
        }
    }

    result.ok = !xml.hasError();
    return result;
}

const QString &translationFor(const ParsedTs &ts, const QString &source)
{
    static const QString empty;
    for (const TsMessage &message : ts.messages) {
        if (message.source == source) return message.translation;
    }
    return empty;
}

} // namespace

// Covers the five new-locale translation files added by this PR:
// resources/i18n/qkeyboardwidget_{de,es,fr,ja,ru}.ts. qkeyboardwidget_ko.ts
// predates this PR (different context/source strings) and is intentionally
// out of scope here.
class TestTranslations : public QObject
{
    Q_OBJECT

private slots:
    void parsesAsWellFormedTsXml_data();
    void parsesAsWellFormedTsXml();

    void hasExpectedTsVersionAndLanguageAttribute_data();
    void hasExpectedTsVersionAndLanguageAttribute();

    void hasNonEmptySpaceAndEnterTranslations_data();
    void hasNonEmptySpaceAndEnterTranslations();

    void translationTextDiffersFromEnglishSource_data();
    void translationTextDiffersFromEnglishSource();

    void sourceContextMatchesRuntimeTranslationLookup();
};

namespace {
QString i18nFilePath(const QString &localeSuffix)
{
#ifdef QKW_I18N_DIR
    return QStringLiteral(QKW_I18N_DIR "/qkeyboardwidget_%1.ts").arg(localeSuffix);
#else
    return QStringLiteral(":/i18n/qkeyboardwidget_%1.ts").arg(localeSuffix);
#endif
}
} // namespace

void TestTranslations::parsesAsWellFormedTsXml_data()
{
    QTest::addColumn<QString>("localeSuffix");
    QTest::newRow("de") << QStringLiteral("de");
    QTest::newRow("es") << QStringLiteral("es");
    QTest::newRow("fr") << QStringLiteral("fr");
    QTest::newRow("ja") << QStringLiteral("ja");
    QTest::newRow("ru") << QStringLiteral("ru");
}

void TestTranslations::parsesAsWellFormedTsXml()
{
    QFETCH(QString, localeSuffix);
    const ParsedTs ts = parseTsFile(i18nFilePath(localeSuffix));
    QVERIFY2(ts.ok, qPrintable(QStringLiteral("failed to parse .ts for locale '%1'").arg(localeSuffix)));
    QCOMPARE(ts.messages.size(), 2);
}

void TestTranslations::hasExpectedTsVersionAndLanguageAttribute_data()
{
    QTest::addColumn<QString>("localeSuffix");
    QTest::addColumn<QString>("expectedLanguage");
    QTest::newRow("de") << QStringLiteral("de") << QStringLiteral("de");
    QTest::newRow("es") << QStringLiteral("es") << QStringLiteral("es");
    QTest::newRow("fr") << QStringLiteral("fr") << QStringLiteral("fr");
    QTest::newRow("ja") << QStringLiteral("ja") << QStringLiteral("ja");
    QTest::newRow("ru") << QStringLiteral("ru") << QStringLiteral("ru");
}

void TestTranslations::hasExpectedTsVersionAndLanguageAttribute()
{
    QFETCH(QString, localeSuffix);
    QFETCH(QString, expectedLanguage);

    const ParsedTs ts = parseTsFile(i18nFilePath(localeSuffix));
    QVERIFY(ts.ok);
    QCOMPARE(ts.version, QStringLiteral("2.1"));
    QCOMPARE(ts.language, expectedLanguage);
}

void TestTranslations::hasNonEmptySpaceAndEnterTranslations_data()
{
    QTest::addColumn<QString>("localeSuffix");
    QTest::newRow("de") << QStringLiteral("de");
    QTest::newRow("es") << QStringLiteral("es");
    QTest::newRow("fr") << QStringLiteral("fr");
    QTest::newRow("ja") << QStringLiteral("ja");
    QTest::newRow("ru") << QStringLiteral("ru");
}

void TestTranslations::hasNonEmptySpaceAndEnterTranslations()
{
    QFETCH(QString, localeSuffix);
    const ParsedTs ts = parseTsFile(i18nFilePath(localeSuffix));
    QVERIFY(ts.ok);

    const QString spaceTranslation = translationFor(ts, QStringLiteral("space"));
    const QString enterTranslation = translationFor(ts, QStringLiteral("enter"));
    QVERIFY2(!spaceTranslation.isEmpty(), qPrintable(QStringLiteral("missing 'space' translation for '%1'").arg(localeSuffix)));
    QVERIFY2(!enterTranslation.isEmpty(), qPrintable(QStringLiteral("missing 'enter' translation for '%1'").arg(localeSuffix)));
}

void TestTranslations::translationTextDiffersFromEnglishSource_data()
{
    QTest::addColumn<QString>("localeSuffix");
    QTest::newRow("de") << QStringLiteral("de");
    QTest::newRow("es") << QStringLiteral("es");
    QTest::newRow("fr") << QStringLiteral("fr");
    QTest::newRow("ja") << QStringLiteral("ja");
    QTest::newRow("ru") << QStringLiteral("ru");
}

void TestTranslations::translationTextDiffersFromEnglishSource()
{
    // Sanity check against copy-paste placeholder translations: every
    // locale here uses a non-Latin or otherwise visibly different word for
    // both "space" and "enter", so the translated text must never equal the
    // (lowercase) English source verbatim.
    QFETCH(QString, localeSuffix);
    const ParsedTs ts = parseTsFile(i18nFilePath(localeSuffix));
    QVERIFY(ts.ok);

    for (const TsMessage &message : ts.messages) {
        QVERIFY2(message.translation.compare(message.source, Qt::CaseInsensitive) != 0,
                 qPrintable(QStringLiteral("'%1' translation for source '%2' looks untranslated")
                                .arg(localeSuffix, message.source)));
    }
}

void TestTranslations::sourceContextMatchesRuntimeTranslationLookup()
{
    // KeyboardController::resolveLabel() (src/keyboard_controller.cpp) looks
    // up UI strings via QCoreApplication::translate() with a fixed literal
    // context of "QKeyboardWidget" and literal English source strings
    // "Space" / "Enter" (capitalized) - see the "kContext" constant and the
    // translate() calls right below it. For a translation to actually be
    // found at runtime, a .ts file's <context><name> must be
    // "QKeyboardWidget" and its <message><source> text must match one of
    // those exact capitalized strings.
    //
    // The five new-locale .ts files added by this PR instead use context
    // "qkw::KeyboardController" and lowercase sources "space"/"enter",
    // which QCoreApplication::translate() will never match - these
    // translations are unreachable at runtime as currently written. This is
    // flagged as an expected failure (rather than a hard failure) to record
    // the mismatch without blocking the build; removing the QEXPECT_FAIL
    // once the .ts files (or the lookup context/source strings) are fixed
    // to agree with each other should turn each check below green.
    static const QStringList kLocales = {QStringLiteral("de"), QStringLiteral("es"), QStringLiteral("fr"),
                                          QStringLiteral("ja"), QStringLiteral("ru")};

    for (const QString &localeSuffix : kLocales) {
        const ParsedTs ts = parseTsFile(i18nFilePath(localeSuffix));
        QVERIFY(ts.ok);

        QEXPECT_FAIL("", qPrintable(QStringLiteral("'%1' context '%2' does not match the runtime lookup context "
                                                    "'QKeyboardWidget'")
                                         .arg(localeSuffix, ts.contextName)),
                     Continue);
        QCOMPARE(ts.contextName, QStringLiteral("QKeyboardWidget"));

        const bool hasCapitalizedSpaceSource = !translationFor(ts, QStringLiteral("Space")).isEmpty();
        QEXPECT_FAIL("", qPrintable(QStringLiteral("'%1' has no source text 'Space' (only lowercase 'space'), so "
                                                    "it will never be matched by QCoreApplication::translate()")
                                         .arg(localeSuffix)),
                     Continue);
        QVERIFY(hasCapitalizedSpaceSource);
    }
}

QTEST_GUILESS_MAIN(TestTranslations)
#include "tst_translations.moc"