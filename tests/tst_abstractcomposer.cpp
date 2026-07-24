#include <QtTest>
#include <QSignalSpy>

#include "qkeyboardwidget/abstract_composer.h"
#include "qkeyboardwidget/hangul_composer.h"
#include "qkeyboardwidget/kana_composer.h"

using namespace qkw;

// AbstractComposer itself is a pure-virtual interface and can't be
// instantiated directly, so these tests exercise its contract through the
// two concrete implementations (HangulComposer, KanaComposer), always going
// through an AbstractComposer* to prove the base interface - not just the
// concrete class API - is fully usable and behaves consistently.
class TestAbstractComposer : public QObject
{
    Q_OBJECT

private slots:
    void hangulComposerIsUsableThroughBasePointer();
    void kanaComposerIsUsableThroughBasePointer();
    void resetThroughBasePointerClearsHangulComposerState();
    void resetThroughBasePointerClearsKanaComposerState();
    void backspaceThroughBasePointerReturnsFalseWhenIdleForBothComposers();
    void syllableClearedSignalIsAccessibleThroughBasePointer();
    void polymorphicDeleteDestroysDerivedInstanceCleanly();
};

void TestAbstractComposer::hangulComposerIsUsableThroughBasePointer()
{
    HangulComposer concrete;
    AbstractComposer *composer = &concrete;

    QSignalSpy spy(composer, &AbstractComposer::syllableReady);
    QVERIFY(!composer->isComposing());

    QVERIFY(composer->feed(QStringLiteral("ㄱ")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ㄱ"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
    QVERIFY(composer->isComposing());

    QVERIFY(composer->backspace());
    QVERIFY(!composer->isComposing());
}

void TestAbstractComposer::kanaComposerIsUsableThroughBasePointer()
{
    KanaComposer concrete;
    AbstractComposer *composer = &concrete;

    QSignalSpy spy(composer, &AbstractComposer::syllableReady);
    QVERIFY(!composer->isComposing());

    QVERIFY(composer->feed(QStringLiteral("か")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("か"));
    QCOMPARE(spy.at(0).at(1).toBool(), false);
    QVERIFY(composer->isComposing());

    QVERIFY(composer->backspace());
    QVERIFY(!composer->isComposing());
}

void TestAbstractComposer::resetThroughBasePointerClearsHangulComposerState()
{
    HangulComposer concrete;
    AbstractComposer *composer = &concrete;
    composer->feed(QStringLiteral("ㄱ"));
    composer->feed(QStringLiteral("ㅏ"));
    QVERIFY(composer->isComposing());

    composer->reset();
    QVERIFY(!composer->isComposing());
}

void TestAbstractComposer::resetThroughBasePointerClearsKanaComposerState()
{
    KanaComposer concrete;
    AbstractComposer *composer = &concrete;
    composer->feed(QStringLiteral("あ"));
    QVERIFY(composer->isComposing());

    composer->reset();
    QVERIFY(!composer->isComposing());
}

void TestAbstractComposer::backspaceThroughBasePointerReturnsFalseWhenIdleForBothComposers()
{
    HangulComposer hangul;
    KanaComposer kana;
    AbstractComposer *composers[] = {&hangul, &kana};
    for (AbstractComposer *composer : composers) {
        QVERIFY(!composer->isComposing());
        QVERIFY(!composer->backspace());
    }
}

void TestAbstractComposer::syllableClearedSignalIsAccessibleThroughBasePointer()
{
    HangulComposer concrete;
    AbstractComposer *composer = &concrete;
    composer->feed(QStringLiteral("ㄱ"));

    QSignalSpy clearedSpy(composer, &AbstractComposer::syllableCleared);
    QVERIFY(composer->backspace());
    QCOMPARE(clearedSpy.count(), 1);
}

void TestAbstractComposer::polymorphicDeleteDestroysDerivedInstanceCleanly()
{
    AbstractComposer *hangul = new HangulComposer;
    AbstractComposer *kana = new KanaComposer;
    hangul->feed(QStringLiteral("ㄱ"));
    kana->feed(QStringLiteral("あ"));

    // Deleting through the abstract base pointer must invoke each derived
    // class's own destructor (QObject's virtual dtor, overridden in
    // AbstractComposer via "~AbstractComposer() override = default")
    // rather than only running ~AbstractComposer() and leaking/UB-ing the
    // derived-class members.
    delete hangul;
    delete kana;
}

QTEST_GUILESS_MAIN(TestAbstractComposer)
#include "tst_abstractcomposer.moc"