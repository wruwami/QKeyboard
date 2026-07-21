#include <QtTest>
#include <QSignalSpy>

#include "qkeyboardwidget/keyboard_controller.h"
#include "qkeyboardwidget/hangul_composer.h"

using namespace qkw;

class TestHangulComposer : public QObject
{
    Q_OBJECT

private slots:
    void nonKoreanLocaleForwardsDirectly();
    void composeSyllables();
    void decomposeOnBackspace();
    void vowelSplittingAndRegrouping();
    void vowelCombinations();
    void consonantCombinations();
    void commitClearsComposingState();
};

void TestHangulComposer::nonKoreanLocaleForwardsDirectly()
{
    KeyboardController controller;
    HangulComposer composer(&controller);

    // Initial state is non-Korean since no layout is loaded
    QVERIFY(!composer.isComposing());

    QSignalSpy charSpy(&composer, &HangulComposer::characterEntered);
    QSignalSpy bsSpy(&composer, &HangulComposer::backspaceRequested);

    // Enter a key: should just forward directly
    emit controller.characterEntered(QStringLiteral("a"));
    QCOMPARE(charSpy.count(), 1);
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("a"));

    // Backspace: should forward directly
    emit controller.backspaceRequested();
    QCOMPARE(bsSpy.count(), 1);
}

void TestHangulComposer::composeSyllables()
{
    KeyboardController controller;
    // Set locale to Korean to activate composition
    QVERIFY(controller.loadJson(R"({"locale":"ko","pages":[{"id":"p","rows":[]}]})"));

    HangulComposer composer(&controller);
    QVERIFY(!composer.isComposing());

    QSignalSpy charSpy(&composer, &HangulComposer::characterEntered);
    QSignalSpy bsSpy(&composer, &HangulComposer::backspaceRequested);

    // 1. Type "ㄱ"
    emit controller.characterEntered(QStringLiteral("ㄱ"));
    QVERIFY(composer.isComposing());
    QCOMPARE(composer.preeditText(), QStringLiteral("ㄱ"));
    QCOMPARE(bsSpy.count(), 0); // No delete sequence needed for first character
    QCOMPARE(charSpy.count(), 1);
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("ㄱ"));

    // 2. Type "ㅏ" -> "가"
    emit controller.characterEntered(QStringLiteral("ㅏ"));
    QVERIFY(composer.isComposing());
    QCOMPARE(composer.preeditText(), QStringLiteral("가"));
    QCOMPARE(bsSpy.count(), 1); // Delete "ㄱ"
    QCOMPARE(charSpy.count(), 1);
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("ga")); // Wait, U+AC00 is "가"
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("가"));

    // 3. Type "ㄴ" -> "간"
    charSpy.clear();
    bsSpy.clear();
    emit controller.characterEntered(QStringLiteral("ㄴ"));
    QVERIFY(composer.isComposing());
    QCOMPARE(composer.preeditText(), QStringLiteral("간"));
    QCOMPARE(bsSpy.count(), 1); // Delete "가"
    QCOMPARE(charSpy.count(), 1);
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("간"));

    // 4. Type "ㅈ" -> "갅"
    charSpy.clear();
    bsSpy.clear();
    emit controller.characterEntered(QStringLiteral("ㅈ"));
    QVERIFY(composer.isComposing());
    QCOMPARE(composer.preeditText(), QStringLiteral("갅"));
    QCOMPARE(bsSpy.count(), 1); // Delete "간"
    QCOMPARE(charSpy.count(), 1);
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("갅"));
}

void TestHangulComposer::decomposeOnBackspace()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(R"({"locale":"ko","pages":[{"id":"p","rows":[]}]})"));

    HangulComposer composer(&controller);

    // Type "ㄱ" + "ㅏ" + "ㄴ" + "ㅈ" -> "갅"
    emit controller.characterEntered(QStringLiteral("ㄱ"));
    emit controller.characterEntered(QStringLiteral("ㅏ"));
    emit controller.characterEntered(QStringLiteral("ㄴ"));
    emit controller.characterEntered(QStringLiteral("ㅈ"));
    QCOMPARE(composer.preeditText(), QStringLiteral("갅"));

    QSignalSpy charSpy(&composer, &HangulComposer::characterEntered);
    QSignalSpy bsSpy(&composer, &HangulComposer::backspaceRequested);

    // Backspace 1: "갅" -> "간" (jong2 'ㅈ' removed)
    emit controller.backspaceRequested();
    QCOMPARE(composer.preeditText(), QStringLiteral("간"));
    QCOMPARE(bsSpy.count(), 1);
    QCOMPARE(charSpy.count(), 1);
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("간"));

    // Backspace 2: "간" -> "가" (jong 'ㄴ' removed)
    charSpy.clear();
    bsSpy.clear();
    emit controller.backspaceRequested();
    QCOMPARE(composer.preeditText(), QStringLiteral("가"));
    QCOMPARE(bsSpy.count(), 1);
    QCOMPARE(charSpy.count(), 1);
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("가"));

    // Backspace 3: "가" -> "ㄱ" (jung 'ㅏ' removed)
    charSpy.clear();
    bsSpy.clear();
    emit controller.backspaceRequested();
    QCOMPARE(composer.preeditText(), QStringLiteral("ㄱ"));
    QCOMPARE(bsSpy.count(), 1);
    QCOMPARE(charSpy.count(), 1);
    QCOMPARE(charSpy.takeFirst().at(0).toString(), QStringLiteral("ㄱ"));

    // Backspace 4: "ㄱ" -> "" (cho 'ㄱ' removed)
    charSpy.clear();
    bsSpy.clear();
    emit controller.backspaceRequested();
    QVERIFY(!composer.isComposing());
    QCOMPARE(composer.preeditText(), QString());
    QCOMPARE(bsSpy.count(), 1);
    QCOMPARE(charSpy.count(), 0);

    // Backspace 5: Forward directly since composing is empty
    charSpy.clear();
    bsSpy.clear();
    emit controller.backspaceRequested();
    QCOMPARE(bsSpy.count(), 1);
    QCOMPARE(charSpy.count(), 0);
}

void TestHangulComposer::vowelSplittingAndRegrouping()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(R"({"locale":"ko","pages":[{"id":"p","rows":[]}]})"));

    HangulComposer composer(&controller);

    // Type "ㄱ" + "ㅏ" + "ㄴ" -> "간"
    emit controller.characterEntered(QStringLiteral("ㄱ"));
    emit controller.characterEntered(QStringLiteral("ㅏ"));
    emit controller.characterEntered(QStringLiteral("ㄴ"));
    QCOMPARE(composer.preeditText(), QStringLiteral("간"));

    QSignalSpy charSpy(&composer, &HangulComposer::characterEntered);
    QSignalSpy bsSpy(&composer, &HangulComposer::backspaceRequested);

    // Type vowel "ㅏ" -> should split: commit "가", preedit "나"
    emit controller.characterEntered(QStringLiteral("ㅏ"));
    QCOMPARE(composer.preeditText(), QStringLiteral("나"));
    QCOMPARE(bsSpy.count(), 1); // Delete "간"
    QCOMPARE(charSpy.count(), 2);
    QCOMPARE(charSpy.at(0).at(0).toString(), QStringLiteral("가"));
    QCOMPARE(charSpy.at(1).at(0).toString(), QStringLiteral("나"));
}

void TestHangulComposer::vowelCombinations()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(R"({"locale":"ko","pages":[{"id":"p","rows":[]}]})"));

    HangulComposer composer(&controller);

    // 1. ㅗ + ㅏ -> ㅘ
    emit controller.characterEntered(QStringLiteral("ㅗ"));
    QCOMPARE(composer.preeditText(), QStringLiteral("ㅗ"));
    emit controller.characterEntered(QStringLiteral("ㅏ"));
    QCOMPARE(composer.preeditText(), QStringLiteral("ㅘ"));

    // Backspace ㅘ -> ㅗ
    emit controller.backspaceRequested();
    QCOMPARE(composer.preeditText(), QStringLiteral("ㅗ"));
    composer.commit();

    // 2. ㅜ + ㅓ -> ㅝ
    emit controller.characterEntered(QStringLiteral("ㅜ"));
    emit controller.characterEntered(QStringLiteral("ㅓ"));
    QCOMPARE(composer.preeditText(), QStringLiteral("ㅝ"));
    composer.commit();

    // 3. ㅡ + ㅣ -> ㅢ
    emit controller.characterEntered(QStringLiteral("ㅡ"));
    emit controller.characterEntered(QStringLiteral("ㅣ"));
    QCOMPARE(composer.preeditText(), QStringLiteral("ㅢ"));
}

void TestHangulComposer::consonantCombinations()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(R"({"locale":"ko","pages":[{"id":"p","rows":[]}]})"));

    HangulComposer composer(&controller);

    // ㄱ + ㅏ + ㄹ + ㄱ -> 갉 (ㄺ final cluster)
    emit controller.characterEntered(QStringLiteral("ㄱ"));
    emit controller.characterEntered(QStringLiteral("ㅏ"));
    emit controller.characterEntered(QStringLiteral("ㄹ"));
    emit controller.characterEntered(QStringLiteral("ㄱ"));
    QCOMPARE(composer.preeditText(), QStringLiteral("갉"));

    // Backspace 갉 -> 갈
    emit controller.backspaceRequested();
    QCOMPARE(composer.preeditText(), QStringLiteral("갈"));
}

void TestHangulComposer::commitClearsComposingState()
{
    KeyboardController controller;
    QVERIFY(controller.loadJson(R"({"locale":"ko","pages":[{"id":"p","rows":[]}]})"));

    HangulComposer composer(&controller);

    emit controller.characterEntered(QStringLiteral("ㄱ"));
    emit controller.characterEntered(QStringLiteral("ㅏ"));
    QVERIFY(composer.isComposing());

    composer.commit();
    QVERIFY(!composer.isComposing());
}

QTEST_GUILESS_MAIN(TestHangulComposer)
#include "tst_hangulcomposer.moc"
