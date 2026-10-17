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

void insertCapability(QVariantMap &result, const QString &key, bool enabled)
{
    result.insert(key, enabled);
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

#ifdef LFQUICK_CAP_PLATFORM_QML
    insertCapability(result, QStringLiteral("module.platformQml"), true);
#else
    insertCapability(result, QStringLiteral("module.platformQml"), false);
#endif

#ifdef LFQUICK_CAP_INTEGRATION
    insertCapability(result, QStringLiteral("module.integration"), true);
#else
    insertCapability(result, QStringLiteral("module.integration"), false);
#endif

#ifdef LFQUICK_CAP_WIDGET_BRIDGE
    insertCapability(result, QStringLiteral("module.widgetBridge"), true);
#else
    insertCapability(result, QStringLiteral("module.widgetBridge"), false);
#endif

#ifdef LFQUICK_CAP_CHARTS
    insertCapability(result, QStringLiteral("module.charts"), true);
#else
    insertCapability(result, QStringLiteral("module.charts"), false);
#endif

#ifdef LFQUICK_CAP_DIAGNOSTICS
    insertCapability(result, QStringLiteral("module.diagnostics"), true);
#else
    insertCapability(result, QStringLiteral("module.diagnostics"), false);
#endif

#ifdef LFQUICK_CAP_QT_GRAPHS
    insertCapability(result, QStringLiteral("adapter.qtGraphs"), true);
#else
    insertCapability(result, QStringLiteral("adapter.qtGraphs"), false);
#endif

#ifdef LFQUICK_CAP_QRANGE_MODEL
    insertCapability(result, QStringLiteral("adapter.rangeModel"), true);
#else
    insertCapability(result, QStringLiteral("adapter.rangeModel"), false);
#endif

#ifdef LFQUICK_CAP_CANVAS_PAINTER
    insertCapability(result, QStringLiteral("adapter.canvasPainter"), true);
#else
    insertCapability(result, QStringLiteral("adapter.canvasPainter"), false);
#endif

#ifdef LFQUICK_CAP_SAFEAREA
    insertCapability(result, QStringLiteral("adapter.safeArea"), true);
#else
    insertCapability(result, QStringLiteral("adapter.safeArea"), false);
#endif

#ifdef LFQUICK_CAP_CUSTOM_QUICK_CHARTS
    insertCapability(result, QStringLiteral("adapter.customQuickCharts"), true);
#else
    insertCapability(result, QStringLiteral("adapter.customQuickCharts"), false);
#endif

#ifdef LFQUICK_CAP_LEGACY_QCUSTOMPLOT
    insertCapability(result, QStringLiteral("adapter.legacyQCustomPlot"), true);
#else
    insertCapability(result, QStringLiteral("adapter.legacyQCustomPlot"), false);
#endif

#ifdef LFQUICK_CAP_STYLEKIT
    insertCapability(result, QStringLiteral("adapter.styleKit"), true);
    insertCapability(result, QStringLiteral("style.styleKit"), true);
#else
    insertCapability(result, QStringLiteral("adapter.styleKit"), false);
    insertCapability(result, QStringLiteral("style.styleKit"), false);
#endif

#ifdef LFQUICK_CAP_FLUENT_STYLE
    insertCapability(result, QStringLiteral("style.fluent"), true);
#else
    insertCapability(result, QStringLiteral("style.fluent"), false);
#endif

#ifdef LFQUICK_CAP_BRAND_TEST_STYLE
    insertCapability(result, QStringLiteral("style.brandTest"), true);
#else
    insertCapability(result, QStringLiteral("style.brandTest"), false);
#endif

#ifdef LFQUICK_CAP_SCREENSHOT_TOOL
    insertCapability(result, QStringLiteral("tooling.screenshot"), true);
#else
    insertCapability(result, QStringLiteral("tooling.screenshot"), false);
#endif

#ifdef LFQUICK_CAP_HOT_RELOAD
    insertCapability(result, QStringLiteral("tooling.hotReload"), true);
#else
    insertCapability(result, QStringLiteral("tooling.hotReload"), false);
#endif

    insertCapability(result, QStringLiteral("qt.supportsSafeArea"), compiledWithVersion(6, 9));
    insertCapability(result, QStringLiteral("qt.supportsRangeModel"), compiledWithVersion(6, 10));
    insertCapability(result, QStringLiteral("qt.supportsCanvasPainter"), compiledWithVersion(6, 11));
    insertCapability(result, QStringLiteral("qt.supportsCanvas2D"), compiledWithVersion(6, 12));
    insertCapability(result, QStringLiteral("qt.supportsQmlPreview"), compiledWithVersion(6, 12));
    insertCapability(result, QStringLiteral("qt.supportsStyleKit"), compiledWithVersion(6, 12));
    return result;
}

bool RuntimeCapabilities::hasCapability(const QString &id) const
{
    return capabilities().value(id.trimmed()).toBool();
}

} // namespace LfQuick
