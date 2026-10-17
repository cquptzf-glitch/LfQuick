#include <LfQuick/Platform/RuntimeCapabilities.h>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QtGlobal>

namespace LfQuick {

namespace {

constexpr bool compiledWithVersion(int major, int minor, int patch = 0)
{
    return QT_VERSION >= QT_VERSION_CHECK(major, minor, patch);
}

} // namespace

RuntimeCapabilities::RuntimeCapabilities(QObject *parent)
    : QObject(parent)
{
}

QString RuntimeCapabilities::qtVersion() const
{
    return QString::fromLatin1(qVersion());
}

QString RuntimeCapabilities::platformName() const
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    return app ? QGuiApplication::platformName() : QString{};
}

bool RuntimeCapabilities::debugBuild() const noexcept
{
#ifdef QT_DEBUG
    return true;
#else
    return false;
#endif
}

QVariantMap RuntimeCapabilities::capabilities() const
{
    QVariantMap result;

#ifdef LFQUICK_CAP_WIDGET_BRIDGE
    result.insert(QStringLiteral("module.widgetBridge"), true);
#else
    result.insert(QStringLiteral("module.widgetBridge"), false);
#endif

#ifdef LFQUICK_CAP_CHARTS
    result.insert(QStringLiteral("module.charts"), true);
#else
    result.insert(QStringLiteral("module.charts"), false);
#endif

#ifdef LFQUICK_CAP_QT_GRAPHS
    result.insert(QStringLiteral("adapter.qtGraphs"), true);
#else
    result.insert(QStringLiteral("adapter.qtGraphs"), false);
#endif

#ifdef LFQUICK_CAP_QRANGE_MODEL
    result.insert(QStringLiteral("adapter.rangeModel"), true);
#else
    result.insert(QStringLiteral("adapter.rangeModel"), false);
#endif

#ifdef LFQUICK_CAP_CANVAS_PAINTER
    result.insert(QStringLiteral("adapter.canvasPainter"), true);
#else
    result.insert(QStringLiteral("adapter.canvasPainter"), false);
#endif

#ifdef LFQUICK_CAP_DIAGNOSTICS
    result.insert(QStringLiteral("module.diagnostics"), true);
#else
    result.insert(QStringLiteral("module.diagnostics"), false);
#endif

#ifdef LFQUICK_CAP_SCREENSHOT_TOOL
    result.insert(QStringLiteral("tooling.screenshot"), true);
#else
    result.insert(QStringLiteral("tooling.screenshot"), false);
#endif

#ifdef LFQUICK_CAP_HOT_RELOAD
    result.insert(QStringLiteral("tooling.hotReload"), true);
#else
    result.insert(QStringLiteral("tooling.hotReload"), false);
#endif

#ifdef LFQUICK_CAP_STYLEKIT
    result.insert(QStringLiteral("adapter.styleKit"), true);
#else
    result.insert(QStringLiteral("adapter.styleKit"), false);
#endif

    result.insert(QStringLiteral("qt.supportsSafeArea"), compiledWithVersion(6, 9));
    result.insert(QStringLiteral("qt.supportsRangeModel"), compiledWithVersion(6, 10));
    result.insert(QStringLiteral("qt.supportsCanvasPainter"), compiledWithVersion(6, 11));
    result.insert(QStringLiteral("qt.supportsCanvas2D"), compiledWithVersion(6, 12));
    result.insert(QStringLiteral("qt.supportsQmlPreview"), compiledWithVersion(6, 12));
    result.insert(QStringLiteral("qt.supportsStyleKit"), compiledWithVersion(6, 12));
    return result;
}

bool RuntimeCapabilities::hasCapability(const QString &id) const
{
    return capabilities().value(id.trimmed()).toBool();
}

} // namespace LfQuick
