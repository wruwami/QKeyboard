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
    void compoundVowelDecomposesOnBackspace();
    void resetClearsState();
    void pullsBackSingleJongseongWhenVowelFollows();
    void explicitCommitClearsState();
    void feedHandlesEmptyString();
    void testsAdditionalCompoundJamoCases();
    void testsAllCompoundVowelsAndDecomposition();
    void testsAllCompoundJongseongAndDecomposition();
    void testsAllSingleJongseongPullbacks();
    void testsAllCompoundJongseongPullbacks();
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

void TestHangulComposer::compoundVowelDecomposesOnBackspace()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅗ"));
    composer.feed(QStringLiteral("ㅏ")); // "과"

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.backspace()); // "고" (ㅗ)
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("고"));

    QVERIFY(composer.backspace()); // "ㄱ"
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toString(), QStringLiteral("ㄱ"));
}

void TestHangulComposer::resetClearsState()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));
    QVERIFY(composer.isComposing());

    QVERIFY(composer.backspace());
    QVERIFY(composer.backspace());
    QVERIFY(!composer.isComposing());
}

void TestHangulComposer::pullsBackSingleJongseongWhenVowelFollows()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));
    composer.feed(QStringLiteral("ㄴ")); // "간"

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("ㅣ"))); // "가" + "니"
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("가"));
    QCOMPARE(spy.at(0).at(1).toBool(), true);
    QCOMPARE(spy.at(1).at(0).toString(), QStringLiteral("니"));
    QCOMPARE(spy.at(1).at(1).toBool(), false);
}

void TestHangulComposer::explicitCommitClearsState()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    composer.feed(QStringLiteral("ㅏ"));
    QVERIFY(composer.isComposing());

    composer.commit();
    QVERIFY(!composer.isComposing());

    QSignalSpy spy(&composer, &HangulComposer::syllableReady);
    QVERIFY(composer.feed(QStringLiteral("ㄴ")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ㄴ"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
}

void TestHangulComposer::feedHandlesEmptyString()
{
    HangulComposer composer;
    composer.feed(QStringLiteral("ㄱ"));
    QVERIFY(composer.isComposing());

    QVERIFY(!composer.feed(QStringLiteral("")));
    QVERIFY(!composer.isComposing());
}

void TestHangulComposer::testsAdditionalCompoundJamoCases()
{
    // Compound vowels: ㅗ+ㅐ -> ㅙ ("돼"), ㅡ+ㅣ -> ㅢ ("의")
    {
        HangulComposer composer;
        composer.feed(QStringLiteral("ㄷ"));
        composer.feed(QStringLiteral("ㅗ"));
        QSignalSpy spy(&composer, &HangulComposer::syllableReady);
        QVERIFY(composer.feed(QStringLiteral("ㅐ")));
        QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("돼"));
    }
    {
        HangulComposer composer;
        composer.feed(QStringLiteral("ㅇ"));
        composer.feed(QStringLiteral("ㅡ"));
        QSignalSpy spy(&composer, &HangulComposer::syllableReady);
        QVERIFY(composer.feed(QStringLiteral("ㅣ")));
        QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("의"));
    }
    // Compound jongseong: ㄱ+ㅅ -> ㄳ ("몫"), ㄴ+ㅈ -> ㄵ ("앉")
    {
        HangulComposer composer;
        composer.feed(QStringLiteral("ㅁ"));
        composer.feed(QStringLiteral("ㅗ"));
        composer.feed(QStringLiteral("ㄱ"));
        QSignalSpy spy(&composer, &HangulComposer::syllableReady);
        QVERIFY(composer.feed(QStringLiteral("ㅅ")));
        QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("몫"));
    }
    {
        HangulComposer composer;
        composer.feed(QStringLiteral("ㅇ"));
        composer.feed(QStringLiteral("ㅏ"));
        composer.feed(QStringLiteral("ㄴ"));
        QSignalSpy spy(&composer, &HangulComposer::syllableReady);
        QVERIFY(composer.feed(QStringLiteral("ㅈ")));
        QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("앉"));
    }
}

void TestHangulComposer::testsAllCompoundVowelsAndDecomposition()
{
    // Test composition and backspace decomposition for all compound vowels:
    // ㅗ+ㅐ -> ㅙ ("돼" -> "도")
    // ㅗ+ㅣ -> ㅚ ("외" -> "오")
    // ㅜ+ㅓ -> ㅝ ("워" -> "우")
    // ㅜ+ㅔ -> ㅞ ("웨" -> "우")
    // ㅜ+ㅣ -> ㅟ ("위" -> "우")
    // ㅡ+ㅣ -> ㅢ ("의" -> "으")
    struct CompoundVowelTest
    {
        QString initialCho;
        QString v1;
        QString v2;
        QString expectedComposed;
        QString expectedDecomposed;
    } tests[] = {
        {QStringLiteral("ㄱ"), QStringLiteral("ㅗ"), QStringLiteral("ㅏ"), QStringLiteral("과"), QStringLiteral("고")},
        {QStringLiteral("ㄷ"), QStringLiteral("ㅗ"), QStringLiteral("ㅐ"), QStringLiteral("돼"), QStringLiteral("도")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅗ"), QStringLiteral("ㅣ"), QStringLiteral("외"), QStringLiteral("오")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅜ"), QStringLiteral("ㅓ"), QStringLiteral("워"), QStringLiteral("우")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅜ"), QStringLiteral("ㅔ"), QStringLiteral("웨"), QStringLiteral("우")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅜ"), QStringLiteral("ㅣ"), QStringLiteral("위"), QStringLiteral("우")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅡ"), QStringLiteral("ㅣ"), QStringLiteral("의"), QStringLiteral("으")},
    };

    for (const auto &t : tests) {
        HangulComposer composer;
        composer.feed(t.initialCho);
        composer.feed(t.v1);
        QSignalSpy spy(&composer, &HangulComposer::syllableReady);
        QVERIFY(composer.feed(t.v2));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), t.expectedComposed);
        QCOMPARE(spy.at(0).at(1).toBool(), true);

        QVERIFY(composer.backspace());
        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.at(1).at(0).toString(), t.expectedDecomposed);
        QCOMPARE(spy.at(1).at(1).toBool(), true);
    }
}

void TestHangulComposer::testsAllCompoundJongseongAndDecomposition()
{
    // Test composition and backspace decomposition for all compound jongseong:
    // ㄱ+ㅅ -> ㄳ ("몫" -> "목")
    // ㄴ+ㅈ -> ㄵ ("앉" -> "안")
    // ㄴ+ㅎ -> ㄶ ("않" -> "안")
    // ㄹ+ㄱ -> ㄺ ("닭" -> "달")
    // ㄹ+ㅁ -> ㄻ ("삶" -> "살")
    // ㄹ+ㅂ -> ㄼ ("밟" -> "발")
    // ㄹ+ㅅ -> ㄽ ("돐" -> "돌")
    // ㄹ+ㅌ -> ㄾ ("핥" -> "할")
    // ㄹ+ㅍ -> ㄿ ("읊" -> "을")
    // ㄹ+ㅎ -> ㅀ ("잃" -> "일")
    // ㅂ+ㅅ -> ㅄ ("값" -> "갑")
    struct CompoundJongseongTest
    {
        QString cho;
        QString jung;
        QString j1;
        QString j2;
        QString expectedComposed;
        QString expectedDecomposed;
    } tests[] = {
        {QStringLiteral("ㅁ"), QStringLiteral("ㅗ"), QStringLiteral("ㄱ"), QStringLiteral("ㅅ"), QStringLiteral("몫"),
         QStringLiteral("목")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅏ"), QStringLiteral("ㄴ"), QStringLiteral("ㅈ"), QStringLiteral("앉"),
         QStringLiteral("안")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅏ"), QStringLiteral("ㄴ"), QStringLiteral("ㅎ"), QStringLiteral("않"),
         QStringLiteral("안")},
        {QStringLiteral("ㄷ"), QStringLiteral("ㅏ"), QStringLiteral("ㄹ"), QStringLiteral("ㄱ"), QStringLiteral("닭"),
         QStringLiteral("달")},
        {QStringLiteral("ㅅ"), QStringLiteral("ㅏ"), QStringLiteral("ㄹ"), QStringLiteral("ㅁ"), QStringLiteral("삶"),
         QStringLiteral("살")},
        {QStringLiteral("ㅂ"), QStringLiteral("ㅏ"), QStringLiteral("ㄹ"), QStringLiteral("ㅂ"), QStringLiteral("밟"),
         QStringLiteral("발")},
        {QStringLiteral("ㄷ"), QStringLiteral("ㅗ"), QStringLiteral("ㄹ"), QStringLiteral("ㅅ"), QStringLiteral("돐"),
         QStringLiteral("돌")},
        {QStringLiteral("ㅎ"), QStringLiteral("ㅏ"), QStringLiteral("ㄹ"), QStringLiteral("ㅌ"), QStringLiteral("핥"),
         QStringLiteral("할")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅡ"), QStringLiteral("ㄹ"), QStringLiteral("ㅍ"), QStringLiteral("읊"),
         QStringLiteral("을")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅣ"), QStringLiteral("ㄹ"), QStringLiteral("ㅎ"), QStringLiteral("잃"),
         QStringLiteral("일")},
        {QStringLiteral("ㄱ"), QStringLiteral("ㅏ"), QStringLiteral("ㅂ"), QStringLiteral("ㅅ"), QStringLiteral("값"),
         QStringLiteral("갑")},
    };

    for (const auto &t : tests) {
        HangulComposer composer;
        composer.feed(t.cho);
        composer.feed(t.jung);
        composer.feed(t.j1);
        QSignalSpy spy(&composer, &HangulComposer::syllableReady);
        QVERIFY(composer.feed(t.j2));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), t.expectedComposed);
        QCOMPARE(spy.at(0).at(1).toBool(), true);

        QVERIFY(composer.backspace());
        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.at(1).at(0).toString(), t.expectedDecomposed);
        QCOMPARE(spy.at(1).at(1).toBool(), true);
    }
}

void TestHangulComposer::testsAllSingleJongseongPullbacks()
{
    // Test single jongseong pullback when followed by a vowel for all valid single finals:
    // ㄲ: "꺾" + "ㅣ" -> "꺼"+"끼"
    // ㄷ: "닫" + "ㅏ" -> "다"+"다"
    // ㅁ: "곰" + "ㅣ" -> "고"+"미"
    // ㅅ: "옷" + "ㅣ" -> "오"+"시"
    // ㅆ: "있" + "ㅓ" -> "이"+"써"
    // ㅇ: "강" + "ㅏ" -> "가"+"아"
    // ㅈ: "낮" + "ㅏ" -> "나"+"자"
    // ㅊ: "꽃" + "ㅣ" -> "꼬"+"치"
    // ㅋ: "엌" + "ㅣ" -> "어"+"키"
    // ㅌ: "밭" + "ㅔ" -> "바"+"테"
    // ㅍ: "잎" + "ㅣ" -> "이"+"피"
    // ㅎ: "좋" + "ㅏ" -> "조"+"하"
    struct SinglePullbackTest
    {
        QString cho;
        QString jung;
        QString jong;
        QString nextVowel;
        QString expectedEmittedReplaced;
        QString expectedEmittedNew;
    } tests[] = {
        {QStringLiteral("ㄲ"), QStringLiteral("ㅓ"), QStringLiteral("ㄲ"), QStringLiteral("ㅣ"), QStringLiteral("꺼"),
         QStringLiteral("끼")},
        {QStringLiteral("ㄷ"), QStringLiteral("ㅏ"), QStringLiteral("ㄷ"), QStringLiteral("ㅏ"), QStringLiteral("다"),
         QStringLiteral("다")},
        {QStringLiteral("ㄱ"), QStringLiteral("ㅗ"), QStringLiteral("ㅁ"), QStringLiteral("ㅣ"), QStringLiteral("고"),
         QStringLiteral("미")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅗ"), QStringLiteral("ㅅ"), QStringLiteral("ㅣ"), QStringLiteral("오"),
         QStringLiteral("시")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅣ"), QStringLiteral("ㅆ"), QStringLiteral("ㅓ"), QStringLiteral("이"),
         QStringLiteral("써")},
        {QStringLiteral("ㄱ"), QStringLiteral("ㅏ"), QStringLiteral("ㅇ"), QStringLiteral("ㅏ"), QStringLiteral("가"),
         QStringLiteral("아")},
        {QStringLiteral("ㄴ"), QStringLiteral("ㅏ"), QStringLiteral("ㅈ"), QStringLiteral("ㅏ"), QStringLiteral("나"),
         QStringLiteral("자")},
        {QStringLiteral("ㄲ"), QStringLiteral("ㅗ"), QStringLiteral("ㅊ"), QStringLiteral("ㅣ"), QStringLiteral("꼬"),
         QStringLiteral("치")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅓ"), QStringLiteral("ㅋ"), QStringLiteral("ㅣ"), QStringLiteral("어"),
         QStringLiteral("키")},
        {QStringLiteral("ㅂ"), QStringLiteral("ㅏ"), QStringLiteral("ㅌ"), QStringLiteral("ㅔ"), QStringLiteral("바"),
         QStringLiteral("테")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅣ"), QStringLiteral("ㅍ"), QStringLiteral("ㅣ"), QStringLiteral("이"),
         QStringLiteral("피")},
        {QStringLiteral("ㅈ"), QStringLiteral("ㅗ"), QStringLiteral("ㅎ"), QStringLiteral("ㅏ"), QStringLiteral("조"),
         QStringLiteral("하")},
    };

    for (const auto &t : tests) {
        HangulComposer composer;
        composer.feed(t.cho);
        composer.feed(t.jung);
        composer.feed(t.jong);
        QSignalSpy spy(&composer, &HangulComposer::syllableReady);
        QVERIFY(composer.feed(t.nextVowel));
        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.at(0).at(0).toString(), t.expectedEmittedReplaced);
        QCOMPARE(spy.at(0).at(1).toBool(), true);
        QCOMPARE(spy.at(1).at(0).toString(), t.expectedEmittedNew);
        QCOMPARE(spy.at(1).at(1).toBool(), false);
    }
}

void TestHangulComposer::testsAllCompoundJongseongPullbacks()
{
    // Test compound jongseong pullback when followed by a vowel for all compound finals:
    // ㄳ: "몫" + "ㅣ" -> "목"+"시"
    // ㄵ: "앉" + "ㅏ" -> "안"+"자"
    // ㄶ: "않" + "ㅏ" -> "안"+"하"
    // ㄻ: "삶" + "ㅣ" -> "살"+"미"
    // ㄼ: "밟" + "ㅏ" -> "발"+"바"
    // ㄽ: "돐" + "ㅣ" -> "돌"+"시"
    // ㄾ: "핥" + "ㅏ" -> "할"+"타"
    // ㄿ: "읊" + "ㅓ" -> "을"+"퍼"
    // ㅀ: "잃" + "ㅓ" -> "일"+"허"
    struct CompoundPullbackTest
    {
        QString cho;
        QString jung;
        QString j1;
        QString j2;
        QString nextVowel;
        QString expectedEmittedReplaced;
        QString expectedEmittedNew;
    } tests[] = {
        {QStringLiteral("ㅁ"), QStringLiteral("ㅗ"), QStringLiteral("ㄱ"), QStringLiteral("ㅅ"), QStringLiteral("ㅣ"),
         QStringLiteral("목"), QStringLiteral("시")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅏ"), QStringLiteral("ㄴ"), QStringLiteral("ㅈ"), QStringLiteral("ㅏ"),
         QStringLiteral("안"), QStringLiteral("자")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅏ"), QStringLiteral("ㄴ"), QStringLiteral("ㅎ"), QStringLiteral("ㅏ"),
         QStringLiteral("안"), QStringLiteral("하")},
        {QStringLiteral("ㄷ"), QStringLiteral("ㅏ"), QStringLiteral("ㄹ"), QStringLiteral("ㄱ"), QStringLiteral("ㅣ"),
         QStringLiteral("달"), QStringLiteral("기")},
        {QStringLiteral("ㅅ"), QStringLiteral("ㅏ"), QStringLiteral("ㄹ"), QStringLiteral("ㅁ"), QStringLiteral("ㅣ"),
         QStringLiteral("살"), QStringLiteral("미")},
        {QStringLiteral("ㅂ"), QStringLiteral("ㅏ"), QStringLiteral("ㄹ"), QStringLiteral("ㅂ"), QStringLiteral("ㅏ"),
         QStringLiteral("발"), QStringLiteral("바")},
        {QStringLiteral("ㄷ"), QStringLiteral("ㅗ"), QStringLiteral("ㄹ"), QStringLiteral("ㅅ"), QStringLiteral("ㅣ"),
         QStringLiteral("돌"), QStringLiteral("시")},
        {QStringLiteral("ㅎ"), QStringLiteral("ㅏ"), QStringLiteral("ㄹ"), QStringLiteral("ㅌ"), QStringLiteral("ㅏ"),
         QStringLiteral("할"), QStringLiteral("타")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅡ"), QStringLiteral("ㄹ"), QStringLiteral("ㅍ"), QStringLiteral("ㅓ"),
         QStringLiteral("을"), QStringLiteral("퍼")},
        {QStringLiteral("ㅇ"), QStringLiteral("ㅣ"), QStringLiteral("ㄹ"), QStringLiteral("ㅎ"), QStringLiteral("ㅓ"),
         QStringLiteral("일"), QStringLiteral("허")},
        {QStringLiteral("ㄱ"), QStringLiteral("ㅏ"), QStringLiteral("ㅂ"), QStringLiteral("ㅅ"), QStringLiteral("ㅣ"),
         QStringLiteral("갑"), QStringLiteral("시")},
    };

    for (const auto &t : tests) {
        HangulComposer composer;
        composer.feed(t.cho);
        composer.feed(t.jung);
        composer.feed(t.j1);
        composer.feed(t.j2);
        QSignalSpy spy(&composer, &HangulComposer::syllableReady);
        QVERIFY(composer.feed(t.nextVowel));
        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.at(0).at(0).toString(), t.expectedEmittedReplaced);
        QCOMPARE(spy.at(0).at(1).toBool(), true);
        QCOMPARE(spy.at(1).at(0).toString(), t.expectedEmittedNew);
        QCOMPARE(spy.at(1).at(1).toBool(), false);
    }
}

QTEST_GUILESS_MAIN(TestHangulComposer)
#include "tst_hangulcomposer.moc"
