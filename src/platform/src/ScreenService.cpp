#include <LfQuick/Platform/ScreenService.h>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QPoint>
#include <QRect>
#include <QScreen>

namespace LfQuick {

namespace {

QVariantMap rectToMap(const QRect &rect)
{
    return {
        {QStringLiteral("x"), rect.x()},
        {QStringLiteral("y"), rect.y()},
        {QStringLiteral("width"), rect.width()},
        {QStringLiteral("height"), rect.height()}
    };
}

} // namespace

ScreenService::ScreenService(QObject *parent)
    : QObject(parent)
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    if (!app)
        return;

    for (QScreen *screen : app->screens())
        attachScreen(screen);

    connect(app, &QGuiApplication::screenAdded,
            this, [this](QScreen *screen) {
                attachScreen(screen);
                emit screensChanged();
            });
    connect(app, &QGuiApplication::screenRemoved,
            this, [this](QScreen *) { emit screensChanged(); });
    connect(app, &QGuiApplication::primaryScreenChanged,
            this, [this](QScreen *) { emit screensChanged(); });
}

int ScreenService::count() const
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    return app ? app->screens().size() : 0;
}

QVariantList ScreenService::screens() const
{
    QVariantList result;
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    if (!app)
        return result;

    result.reserve(app->screens().size());
    for (QScreen *screen : app->screens())
        result.push_back(screenToMap(screen));
    return result;
}

QVariantMap ScreenService::primaryScreen() const
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    return app ? screenToMap(app->primaryScreen()) : QVariantMap{};
}

QVariantMap ScreenService::screenAt(int x, int y) const
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    return app ? screenToMap(app->screenAt(QPoint(x, y))) : QVariantMap{};
}

QVariantMap ScreenService::screenByName(const QString &name) const
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    if (!app)
        return {};

    for (QScreen *screen : app->screens()) {
        if (screen && screen->name() == name)
            return screenToMap(screen);
    }
    return {};
}

void ScreenService::attachScreen(QScreen *screen)
{
    if (!screen)
        return;

    connect(screen, &QScreen::geometryChanged,
            this, [this](const QRect &) { emit screensChanged(); });
    connect(screen, &QScreen::availableGeometryChanged,
            this, [this](const QRect &) { emit screensChanged(); });
    connect(screen, &QScreen::logicalDotsPerInchChanged,
            this, [this](qreal) { emit screensChanged(); });
    connect(screen, &QScreen::physicalDotsPerInchChanged,
            this, [this](qreal) { emit screensChanged(); });
    connect(screen, &QScreen::orientationChanged,
            this, [this](Qt::ScreenOrientation) { emit screensChanged(); });
    connect(screen, &QScreen::refreshRateChanged,
            this, [this](qreal) { emit screensChanged(); });
}

QVariantMap ScreenService::screenToMap(QScreen *screen) const
{
    if (!screen)
        return {};

    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    return {
        {QStringLiteral("name"), screen->name()},
        {QStringLiteral("manufacturer"), screen->manufacturer()},
        {QStringLiteral("model"), screen->model()},
        {QStringLiteral("serialNumber"), screen->serialNumber()},
        {QStringLiteral("geometry"), rectToMap(screen->geometry())},
        {QStringLiteral("availableGeometry"), rectToMap(screen->availableGeometry())},
        {QStringLiteral("devicePixelRatio"), screen->devicePixelRatio()},
        {QStringLiteral("logicalDotsPerInch"), screen->logicalDotsPerInch()},
        {QStringLiteral("physicalDotsPerInch"), screen->physicalDotsPerInch()},
        {QStringLiteral("refreshRate"), screen->refreshRate()},
        {QStringLiteral("orientation"), int(screen->orientation())},
        {QStringLiteral("primary"), app && app->primaryScreen() == screen}
    };
}

} // namespace LfQuick
