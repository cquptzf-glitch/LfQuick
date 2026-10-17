#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QColor>
#include <QObject>
#include <QString>

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT SystemAppearance : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ColorScheme colorScheme READ colorScheme NOTIFY appearanceChanged FINAL)
    Q_PROPERTY(QString colorSchemeName READ colorSchemeName NOTIFY appearanceChanged FINAL)
    Q_PROPERTY(bool dark READ isDark NOTIFY appearanceChanged FINAL)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY appearanceChanged FINAL)

public:
    enum class ColorScheme {
        Unknown,
        Light,
        Dark
    };
    Q_ENUM(ColorScheme)

    explicit SystemAppearance(QObject *parent = nullptr);

    ColorScheme colorScheme() const;
    QString colorSchemeName() const;
    bool isDark() const;
    QColor accentColor() const;

signals:
    void appearanceChanged();
};

} // namespace LfQuick
