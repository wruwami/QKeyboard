#pragma once

#include <QObject>
#include <QChar>
#include <QString>

namespace qkw {

class KeyboardController;

class HangulComposer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qkw::KeyboardController* controller READ controller WRITE setController NOTIFY controllerChanged)
    Q_PROPERTY(bool composing READ isComposing NOTIFY compositionChanged)
    Q_PROPERTY(QString preeditText READ preeditText NOTIFY compositionChanged)

public:
    explicit HangulComposer(QObject *parent = nullptr);
    explicit HangulComposer(KeyboardController *controller, QObject *parent = nullptr);
    ~HangulComposer() override;

    KeyboardController *controller() const;
    void setController(KeyboardController *controller);

    bool isComposing() const;
    QString preeditText() const;

    Q_INVOKABLE void commit();

signals:
    void characterEntered(const QString &text);
    void backspaceRequested();
    void controllerChanged();
    void compositionChanged();

private slots:
    void handleCharacterEntered(const QString &text);
    void handleBackspaceRequested();
    void handleLayoutChanged();

private:
    struct HangulSyllable {
        QChar cho;
        QChar jung;
        QChar jong;
        QChar jung1;
        QChar jung2;
        QChar jong1;
        QChar jong2;

        void clear() {
            cho = QChar();
            jung = QChar();
            jong = QChar();
            jung1 = QChar();
            jung2 = QChar();
            jong1 = QChar();
            jong2 = QChar();
        }

        bool isEmpty() const {
            return cho.isNull() && jung.isNull();
        }
    };

    bool isConsonant(QChar c) const;
    bool isVowel(QChar c) const;
    QChar combineVowels(QChar v1, QChar v2) const;
    QChar combineConsonants(QChar c1, QChar c2) const;
    int getChoseongIndex(QChar c) const;
    int getJungseongIndex(QChar c) const;
    int getJongseongIndex(QChar c) const;

    QString syllableToString(const HangulSyllable &s) const;
    QString processKey(QChar c);

    KeyboardController *_controller = nullptr;
    HangulSyllable _current;
    bool _isKorean = false;
};

} // namespace qkw
