#ifndef QKW_ABSTRACT_COMPOSER_H
#define QKW_ABSTRACT_COMPOSER_H

#include <QObject>
#include <QString>

namespace qkw {

/**
 * @brief Abstract base class for multi-stage text input composers (IMEs).
 */
class AbstractComposer : public QObject
{
    Q_OBJECT

public:
    explicit AbstractComposer(QObject *parent = nullptr);
    ~AbstractComposer() override = default;

    /**
     * @brief Feed an input string (e.g. key press) to the composer automata.
     * @return true if input was consumed/handled, false otherwise.
     */
    virtual bool feed(const QString &input) = 0;

    /**
     * @brief Perform a backspace operation on the currently composing text.
     * @return true if a character or composition state was removed, false if idle.
     */
    virtual bool backspace() = 0;

    /**
     * @brief Immediately reset and clear all composition internal state.
     */
    virtual void reset() = 0;

    /**
     * @brief Check whether an active composition is currently in progress.
     */
    virtual bool isComposing() const = 0;

signals:
    /**
     * @brief Emitted when a composed syllable/character is ready or updated.
     * @param text The composed character string.
     * @param isUpdate true if replacing the current composing text, false if appending.
     */
    void syllableReady(const QString &text, bool isUpdate);

    /**
     * @brief Emitted when composition was completely erased via backspace or reset.
     */
    void syllableCleared();
};

inline AbstractComposer::AbstractComposer(QObject *parent) : QObject(parent)
{
}

} // namespace qkw

#endif // QKW_ABSTRACT_COMPOSER_H
