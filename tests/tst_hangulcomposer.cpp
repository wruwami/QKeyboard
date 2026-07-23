#include <QtTest>

#include "qkeyboardwidget/hangul_composer.h"

using namespace qkw;

class TestHangulComposer : public QObject
{
    Q_OBJECT

private slots:
    void composesSimpleSyllable();
    void composesSyllableWithFinal();
    void pullsBackJongseongWhenVowelFollows();
    void splitsCompoundJongseongOnPullback();
    void combinesCompoundVowel();
    void nonJamoPassesThroughAndFlushesComposition();
    void backspaceRemovesJongseongFirst();
    void backspaceDecomposesCompoundVowel();
    void backspaceClearsLoneChoseong();
    void backspaceOnIdleComposerReturnsFalse();
    void invalidJongseongCandidateStartsNewSyllable();

    // New tests for issue #82
    void feedRejectsInvalidLength();
    void vowelOnlyTypedInitially();
    void nonJamoInputCommitsAndResets();
    void nonCombinableDoubleJamoStartsNewSyllable();
    void nonCombinableVowelsStartsNewSyllable();
    void multiStepBackspaceDecomposition();
};

// "가": ㄱ (cho 0) + ㅏ (jung 0) -> U+AC00.
void TestHangulComposer::composesSimpleSyllable()
{
    HangulComposer composer;
    QSignalSpy spy(&composer, &HangulComposer::syllableReady);

    QVERIFY(composer.feed(QStringLiteral("ㄱ")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ㄱ"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);

    QVERIFY(composer.feed(QStringLiteral("ㅏ")));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toString(), QString(QChar(0xAC00))); // 가
    QCOMPARE(spy.at(1).at(1).toBool(), true);
    QVERIFY(composer.isComposing());
}

// "간": ㄱ + ㅏ + ㄴ (jong 4) -> U+AC04.
void TestHangulComposer::composesSyllableWithFinal()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("ㄴ")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(QChar(0xAC04))); // 간
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

// Typing ㄷㅏㄹㄱ then ㅣ: 닭 (U+B2ED) pulls its ㄱ back to start 기 (U+AE30),
// leaving 달 (U+B2EC) behind -- i.e. "닭" + "ㅣ" composes as "달기".
void TestHangulComposer::pullsBackJongseongWhenVowelFollows()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄷ"));
    composer.feed(QStringLiteral("ㅏ"));
    composer.feed(QStringLiteral("ㄹ"));
    composer.feed(QStringLiteral("ㄱ"));

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("ㅣ")));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).toString(), QString(QChar(0xB2EC))); // 달, replaces 닭
    QCOMPARE(spy.at(0).at(1).toBool(), true);
    QCOMPARE(spy.at(1).at(0).toString(), QString(QChar(0xAE30))); // 기, new character
    QCOMPARE(spy.at(1).at(1).toBool(), false);
}

void TestHangulComposer::splitsCompoundJongseongOnPullback()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄷ"));
    composer.feed(QStringLiteral("ㅏ"));
    composer.feed(QStringLiteral("ㄹ"));
    QVERIFY(composer.feed(QStringLiteral("ㄱ"))); // combines into ㄺ jongseong

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    composer.feed(QStringLiteral("ㅣ"));
    // First emission (replace) must be the syllable with just ㄹ as final.
    QCOMPARE(spy.at(0).at(0).toString(), QString(QChar(0xB2EC))); // 달
}

// ㅗ + ㅏ -> ㅘ, e.g. ㅎ + ㅗ + ㅏ = 화 (U+D654).
void TestHangulComposer::combinesCompoundVowel()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㅎ"));
    composer.feed(QStringLiteral("ㅗ"));

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("ㅏ")));
    QCOMPARE(spy.at(0).at(0).toString(), QString(QChar(0xD654))); // 화
    QCOMPARE(spy.at(0).at(1).toBool(), true);
}

void TestHangulComposer::nonJamoPassesThroughAndFlushesComposition()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    QVERIFY(composer.isComposing());

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(!composer.feed(QStringLiteral("x")));
    QCOMPARE(spy.count(), 0);
    QVERIFY(!composer.isComposing());
}

void TestHangulComposer::backspaceRemovesJongseongFirst()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));
    composer.feed(QStringLiteral("ㄴ")); // 간

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.backspace());
    QCOMPARE(spy.at(0).at(0).toString(), QString(QChar(0xAC00))); // back to 가
    QVERIFY(composer.isComposing());
}

void TestHangulComposer::backspaceDecomposesCompoundVowel()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㅎ"));
    composer.feed(QStringLiteral("ㅗ"));
    composer.feed(QStringLiteral("ㅏ")); // 화 (ㅎ+ㅘ)

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.backspace());
    // ㅘ backspaces down to ㅗ, i.e. syllable becomes 호.
    QCOMPARE(spy.at(0).at(0).toString(), QString(QChar(0xD638))); // 호
}

void TestHangulComposer::backspaceClearsLoneChoseong()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));

    QSignalSpy clearSpy(&composer, &HangulComposer::syllableCleared);
    QVERIFY(composer.backspace());
    QCOMPARE(clearSpy.count(), 1);
    QVERIFY(!composer.isComposing());
}

void TestHangulComposer::backspaceOnIdleComposerReturnsFalse()
{
    HangulComposer composer;
    QVERIFY(!composer.backspace());
}

// ㄸ/ㅃ/ㅉ can never be a syllable-final, so after "가" one of them must
// start a new syllable rather than being absorbed as jongseong.
void TestHangulComposer::invalidJongseongCandidateStartsNewSyllable()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ")); // 가

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("ㅃ")));
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ㅃ"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
}

void TestHangulComposer::feedRejectsInvalidLength()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    QVERIFY(composer.isComposing());

    QVERIFY(!composer.feed(QStringLiteral("ㄱㅏ")));
    QVERIFY(!composer.isComposing());
}

void TestHangulComposer::vowelOnlyTypedInitially()
{
    HangulComposer composer;
    QSignalSpy spy(&composer, &HangulComposer::syllableReady);

    QVERIFY(composer.feed(QStringLiteral("ㅏ")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ㅏ"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
    QVERIFY(!composer.isComposing());
}

void TestHangulComposer::nonJamoInputCommitsAndResets()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));
    QVERIFY(composer.isComposing());

    QVERIFY(!composer.feed(QStringLiteral("가")));
    QVERIFY(!composer.isComposing());
}

void TestHangulComposer::nonCombinableDoubleJamoStartsNewSyllable()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));
    composer.feed(QStringLiteral("ㄴ"));

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("ㄴ")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ㄴ"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
    QVERIFY(composer.isComposing());

    QVERIFY(composer.feed(QStringLiteral("ㅏ")));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toString(), QStringLiteral("나"));
    QCOMPARE(spy.at(1).at(1).toBool(), true);
}

void TestHangulComposer::nonCombinableVowelsStartsNewSyllable()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("ㅓ")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ㅓ"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
    QVERIFY(!composer.isComposing());
}

void TestHangulComposer::multiStepBackspaceDecomposition()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));
    composer.feed(QStringLiteral("ㅂ"));
    composer.feed(QStringLiteral("ㅅ"));

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QSignalSpy clearSpy(&composer, &HangulComposer::syllableCleared);

    QVERIFY(composer.backspace());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(QChar(0xAC11)));

    QVERIFY(composer.backspace());
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toString(), QString(QChar(0xAC00)));

    QVERIFY(composer.backspace());
    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(2).at(0).toString(), QStringLiteral("ㄱ"));

    QVERIFY(composer.backspace());
    QCOMPARE(clearSpy.count(), 1);
    QVERIFY(!composer.isComposing());
}

QTEST_GUILESS_MAIN(TestHangulComposer)
#include "tst_hangulcomposer.moc"
