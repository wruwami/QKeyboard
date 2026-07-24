#include <QtTest>
#include <QSignalSpy>

#include "qkeyboard/kana_composer.h"

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

    // Additional edge-case coverage.
    void isComposingIsFalseInitially();
    void backspaceOnIdleComposerReturnsFalse();
    void modifierFedWhileIdleReturnsFalse();
    void dakutenOnUnsupportedKanaReturnsFalseAndKeepsState();
    void handakutenOnUnsupportedKanaReturnsFalseAndKeepsState();
    void smallKanaOnUnsupportedKanaReturnsFalseAndKeepsState();
    void alternateDakutenGlyphAppliesSameAsCanonical();
    void alternateHandakutenGlyphAppliesSameAsCanonical();
    void alternateSmallKanaGlyphAppliesSameAsCanonical();
    void feedingNewKanaReplacesPreviousComposingState();
    void backspaceReversesHandakuten();
    void backspaceAfterSmallKanaToggleRestoresNormalKana();
    void emptyStringFeedReturnsFalseAndPreservesState();
    void resetWhileIdleIsANoOp();
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

    composer.reset();
    QVERIFY(!composer.isComposing());
}

void TestKanaComposer::isComposingIsFalseInitially()
{
    KanaComposer composer;
    QVERIFY(!composer.isComposing());
}

void TestKanaComposer::backspaceOnIdleComposerReturnsFalse()
{
    KanaComposer composer;
    QVERIFY(!composer.backspace());
    QVERIFY(!composer.isComposing());
}

void TestKanaComposer::modifierFedWhileIdleReturnsFalse()
{
    KanaComposer composer;

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(!composer.feed(QStringLiteral("゛")));
    QVERIFY(!composer.feed(QStringLiteral("゜")));
    QVERIFY(!composer.feed(QStringLiteral("小")));
    QCOMPARE(spy.count(), 0);
    QVERIFY(!composer.isComposing());
}

void TestKanaComposer::dakutenOnUnsupportedKanaReturnsFalseAndKeepsState()
{
    // "あ" has no dakuten form, so "゛" must be rejected and the composing
    // kana must be left untouched (no signal emitted).
    KanaComposer composer;
    composer.feed(QStringLiteral("あ"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(!composer.feed(QStringLiteral("゛")));
    QCOMPARE(spy.count(), 0);
    QVERIFY(composer.isComposing());
}

void TestKanaComposer::handakutenOnUnsupportedKanaReturnsFalseAndKeepsState()
{
    // "か" has a dakuten form but no handakuten form.
    KanaComposer composer;
    composer.feed(QStringLiteral("か"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(!composer.feed(QStringLiteral("゜")));
    QCOMPARE(spy.count(), 0);
    QVERIFY(composer.isComposing());
}

void TestKanaComposer::smallKanaOnUnsupportedKanaReturnsFalseAndKeepsState()
{
    // "か" has no small-kana form.
    KanaComposer composer;
    composer.feed(QStringLiteral("か"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(!composer.feed(QStringLiteral("小")));
    QCOMPARE(spy.count(), 0);
    QVERIFY(composer.isComposing());
}

void TestKanaComposer::alternateDakutenGlyphAppliesSameAsCanonical()
{
    // "″" is accepted as an alternate spelling of the dakuten mark "゛".
    KanaComposer composer;
    composer.feed(QStringLiteral("か"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("″")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("が"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

void TestKanaComposer::alternateHandakutenGlyphAppliesSameAsCanonical()
{
    // "°" is accepted as an alternate spelling of the handakuten mark "゜".
    KanaComposer composer;
    composer.feed(QStringLiteral("は"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("°")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ぱ"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

void TestKanaComposer::alternateSmallKanaGlyphAppliesSameAsCanonical()
{
    // "小/大" is accepted as an alternate label for the small/large toggle "小".
    KanaComposer composer;
    composer.feed(QStringLiteral("つ"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("小/大")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("っ"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

void TestKanaComposer::feedingNewKanaReplacesPreviousComposingState()
{
    // Feeding a plain kana always starts a fresh composing character,
    // discarding whatever was composing before (no explicit commit needed).
    KanaComposer composer;
    composer.feed(QStringLiteral("あ"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("か")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("か"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
    QVERIFY(composer.isComposing());
}

void TestKanaComposer::backspaceReversesHandakuten()
{
    KanaComposer composer;
    composer.feed(QStringLiteral("は"));
    composer.feed(QStringLiteral("゜")); // "ぱ"

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(composer.backspace());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("は"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
    QVERIFY(composer.isComposing());
}

void TestKanaComposer::backspaceAfterSmallKanaToggleRestoresNormalKana()
{
    // Backspace after a small-kana toggle restores the normal (large) form,
    // just like it does for dakuten/handakuten modifiers.
    KanaComposer composer;
    composer.feed(QStringLiteral("つ"));
    composer.feed(QStringLiteral("小")); // "っ"

    QSignalSpy readySpy(&composer, &KanaComposer::syllableReady);
    QSignalSpy clearedSpy(&composer, &KanaComposer::syllableCleared);
    QVERIFY(composer.backspace());
    QCOMPARE(readySpy.count(), 1);
    QCOMPARE(readySpy.at(0).at(0).toString(), QStringLiteral("つ"));
    QCOMPARE(readySpy.at(0).at(1).toBool(), true);
    QVERIFY(composer.isComposing());
}

void TestKanaComposer::emptyStringFeedReturnsFalseAndPreservesState()
{
    KanaComposer composer;
    composer.feed(QStringLiteral("あ"));

    QSignalSpy spy(&composer, &KanaComposer::syllableReady);
    QVERIFY(!composer.feed(QStringLiteral("")));
    QCOMPARE(spy.count(), 0);
    QVERIFY(composer.isComposing());
}

void TestKanaComposer::resetWhileIdleIsANoOp()
{
    KanaComposer composer;
    QSignalSpy spy(&composer, &KanaComposer::syllableCleared);
    composer.reset();
    QCOMPARE(spy.count(), 0);
    QVERIFY(!composer.isComposing());
}

QTEST_GUILESS_MAIN(TestKanaComposer)
#include "tst_kanacomposer.moc"
