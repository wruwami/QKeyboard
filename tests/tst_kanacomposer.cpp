#include <QtTest>
#include <QSignalSpy>

#include "qkeyboardwidget/kana_composer.h"

using namespace qkw;

class TestKanaComposer : public QObject
{
    Q_OBJECT

private slots:
    void feedsKanaAndEmitsSignal();
    void appliesDakutenToSupportedKana();
    void appliesHandakutenToSupportedKana();
    void togglesSmallKana();
    void backspaceReversesModifiers();
    void backspaceClearsLoneKana();
    void resetClearsState();
    void feedEmptyOrUnsupportedModifiersReturnsFalse();
    void backspaceReversesHandakutenAndIdleReturnsFalse();
};

void TestKanaComposer::feedsKanaAndEmitsSignal()
{
    KanaComposer composer;
    QSignalSpy spy(&composer, &KanaComposer::syllableReady);

    QVERIFY(composer.feed(QStringLiteral("か")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("か"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
    QVERIFY(composer.isComposing());
}

void TestKanaComposer::appliesDakutenToSupportedKana()
{
    KanaComposer composer;
    composer.feed(QStringLiteral("か"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("゛")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("が"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

void TestKanaComposer::appliesHandakutenToSupportedKana()
{
    KanaComposer composer;
    composer.feed(QStringLiteral("は"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("゜")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ぱ"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

void TestKanaComposer::togglesSmallKana()
{
    KanaComposer composer;
    composer.feed(QStringLiteral("つ"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("小")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("っ"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

void TestKanaComposer::backspaceReversesModifiers()
{
    KanaComposer composer;
    composer.feed(QStringLiteral("か"));
    composer.feed(QStringLiteral("゛"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.backspace());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("か"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

void TestKanaComposer::backspaceClearsLoneKana()
{
    KanaComposer composer;
    composer.feed(QStringLiteral("あ"));

    QSignalSpy spy(&composer, &KanaComposer::syllableCleared);
    QVERIFY(composer.backspace());
    QCOMPARE(spy.count(), 1);
    QVERIFY(!composer.isComposing());
}

void TestKanaComposer::resetClearsState()
{
    KanaComposer composer;
    composer.feed(QStringLiteral("あ"));
    QVERIFY(composer.isComposing());

    QSignalSpy spy(&composer, &KanaComposer::syllableCleared);
    composer.reset();
    QVERIFY(!composer.isComposing());
    QCOMPARE(spy.count(), 1);
}

void TestKanaComposer::feedEmptyOrUnsupportedModifiersReturnsFalse()
{
    KanaComposer composer;
    QVERIFY(!composer.feed(QString()));
    QVERIFY(!composer.feed(QStringLiteral("゛")));
    QVERIFY(!composer.feed(QStringLiteral("゜")));
    QVERIFY(!composer.feed(QStringLiteral("小")));

    composer.feed(QStringLiteral("あ"));
    QVERIFY(!composer.feed(QStringLiteral("゛")));
    QVERIFY(!composer.feed(QStringLiteral("゜")));
}

void TestKanaComposer::backspaceReversesHandakutenAndIdleReturnsFalse()
{
    KanaComposer composer;
    QVERIFY(!composer.backspace());

    composer.feed(QStringLiteral("は"));
    composer.feed(QStringLiteral("゜"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.backspace());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("は"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

QTEST_GUILESS_MAIN(TestKanaComposer)
#include "tst_kanacomposer.moc"
