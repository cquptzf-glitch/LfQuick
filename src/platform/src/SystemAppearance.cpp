#include <LfQuick/Platform/SystemAppearance.h>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

namespace LfQuick {

SystemAppearance::SystemAppearance(QObject *parent)
    : QObject(parent)
{
    if (auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
        connect(app, &QGuiApplication::paletteChanged,
                this, [this](const QPalette &) { emit appearanceChanged(); });

        if (auto *hints = app->styleHints()) {
            connect(hints, &QStyleHints::colorSchemeChanged,
                    this, [this](Qt::ColorScheme) { emit appearanceChanged(); });
        }
    }
}

SystemAppearance::ColorScheme SystemAppearance::colorScheme() const
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    if (!app || !app->styleHints())
        return ColorScheme::Unknown;

    switch (app->styleHints()->colorScheme()) {
    case Qt::ColorScheme::Light:
        return ColorScheme::Light;
    case Qt::ColorScheme::Dark:
        return ColorScheme::Dark;
    case Qt::ColorScheme::Unknown:
    default:
        return ColorScheme::Unknown;
    }
}

QString SystemAppearance::colorSchemeName() const
{
    switch (colorScheme()) {
    case ColorScheme::Light:
        return QStringLiteral("light");
    case ColorScheme::Dark:
        return QStringLiteral("dark");
    case ColorScheme::Unknown:
    default:
        return QStringLiteral("unknown");
    }
}

bool SystemAppearance::isDark() const
{
    return colorScheme() == ColorScheme::Dark;
}

QColor SystemAppearance::accentColor() const
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    return app ? app->palette().color(QPalette::Highlight) : QColor{};
}

} // namespace LfQuick
