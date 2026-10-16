#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QVariantMap>
#include <QtQml/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT ValidationEvaluator : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ValidationEvaluator(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap validate(const QVariant &value,
                                     const QVariantMap &rule) const;
    Q_INVOKABLE bool isValid(const QVariant &value,
                             const QVariantMap &rule) const;
};

} // namespace LfQuick
