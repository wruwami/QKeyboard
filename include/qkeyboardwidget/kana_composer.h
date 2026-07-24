#ifndef QKW_KANA_COMPOSER_H
#define QKW_KANA_COMPOSER_H

#include "qkeyboardwidget/abstract_composer.h"
#include "qkeyboardwidget/qkw_export.h"

namespace qkw {

/**
 * @brief Handles Japanese Kana input transitions such as Dakuten (濁点),
 * Handakuten (半濁点), and Small Kana (小文字) conversions.
 */
class QKW_EXPORT KanaComposer : public AbstractComposer
{
    Q_OBJECT

public:
    explicit KanaComposer(QObject *parent = nullptr);

    bool feed(const QString &text) override;
    bool backspace() override;
    void reset() override;
    bool isComposing() const override;

private:
    // Which modifier (if any) produced _currentKana's current value, tracked
    // explicitly rather than inferred by reverse-searching the modifier maps
    // for a match: smallKanaMap() is bidirectional (large<->small, so "小"
    // can toggle either way), so a value like plain "あ" is *also* the
    // target of a small-kana reverse entry ("ぁ" -> "あ") purely by
    // coincidence - reverse-lookup alone can't tell "reached via toggle"
    // apart from "typed directly and happens to have a small-kana
    // counterpart". backspace() needs to know which actually happened.
    enum class LastModifier { None, Dakuten, Handakuten, SmallKana };

    QString _currentKana;
    LastModifier _lastModifier = LastModifier::None;
};

} // namespace qkw

#endif // QKW_KANA_COMPOSER_H
