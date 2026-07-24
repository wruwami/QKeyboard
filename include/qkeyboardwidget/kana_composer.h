#ifndef QKW_KANA_COMPOSER_H
#define QKW_KANA_COMPOSER_H

#include "qkeyboardwidget/abstract_composer.h"
#include "qkeyboardwidget/qkw_export.h"

namespace qkw {

/**
 * @brief Handles Japanese Kana input transitions such as Dakuten (濁点), Handakuten (半濁点), and Small Kana (小文字) conversions.
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
    QString _currentKana;
};

} // namespace qkw

#endif // QKW_KANA_COMPOSER_H
