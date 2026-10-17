#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QQmlEngine>
#include <QQuickView>
#include <QTimer>

namespace {

QUrl sourceUrl(const QString &value)
{
    const QString trimmed = value.trimmed();
    if (trimmed.startsWith(QStringLiteral(":/")))
        return QUrl(QStringLiteral("qrc") + trimmed);

    const QUrl parsed(trimmed);
    if (parsed.isValid() && !parsed.scheme().isEmpty())
        return parsed;

    return QUrl::fromLocalFile(QFileInfo(trimmed).absoluteFilePath());
}

int positiveInt(const QString &value, int fallback)
{
    bool ok = false;
    const int parsed = value.toInt(&ok);
    return ok && parsed > 0 ? parsed : fallback;
}

} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("lfquick-screenshot"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Render a QML Item root and save it as a PNG screenshot."));
    parser.addHelpOption();

    const QCommandLineOption sourceOption(
        {QStringLiteral("s"), QStringLiteral("source")},
        QStringLiteral("QML file or URL. The root object must be a QQuickItem."),
        QStringLiteral("url"));
    const QCommandLineOption outputOption(
        {QStringLiteral("o"), QStringLiteral("output")},
        QStringLiteral("Output PNG path."),
        QStringLiteral("png"));
    const QCommandLineOption widthOption(
        QStringLiteral("width"), QStringLiteral("Viewport width."), QStringLiteral("pixels"), QStringLiteral("1280"));
    const QCommandLineOption heightOption(
        QStringLiteral("height"), QStringLiteral("Viewport height."), QStringLiteral("pixels"), QStringLiteral("720"));
    const QCommandLineOption delayOption(
        QStringLiteral("delay"), QStringLiteral("Delay after QML becomes ready before capture."),
        QStringLiteral("milliseconds"), QStringLiteral("250"));
    const QCommandLineOption importPathOption(
        {QStringLiteral("I"), QStringLiteral("import-path")},
        QStringLiteral("Additional QML import path. May be specified multiple times."),
        QStringLiteral("path"));

    parser.addOption(sourceOption);
    parser.addOption(outputOption);
    parser.addOption(widthOption);
    parser.addOption(heightOption);
    parser.addOption(delayOption);
    parser.addOption(importPathOption);
    parser.process(app);

    if (!parser.isSet(sourceOption) || !parser.isSet(outputOption))
        parser.showHelp(1);

    const QUrl source = sourceUrl(parser.value(sourceOption));
    if (!source.isValid() || source.isEmpty()) {
        qCritical("Invalid QML source");
        return 2;
    }

    const QFileInfo outputInfo(parser.value(outputOption));
    const QString outputPath = outputInfo.absoluteFilePath();
    QDir().mkpath(outputInfo.absolutePath());

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.resize(positiveInt(parser.value(widthOption), 1280),
                positiveInt(parser.value(heightOption), 720));

    for (const QString &path : parser.values(importPathOption)) {
        if (!path.trimmed().isEmpty())
            view.engine()->addImportPath(QFileInfo(path).absoluteFilePath());
    }

    bool captureScheduled = false;
    const auto scheduleCapture = [&] {
        if (captureScheduled || view.status() != QQuickView::Ready)
            return;
        captureScheduled = true;
        view.show();

        const int delay = qMax(0, parser.value(delayOption).toInt());
        QTimer::singleShot(delay, &app, [&] {
            const QImage image = view.grabWindow();
            if (image.isNull()) {
                qCritical("Screenshot capture returned an empty image");
                app.exit(3);
                return;
            }
            if (!image.save(outputPath, "PNG")) {
                qCritical("Failed to save screenshot to %s", qPrintable(outputPath));
                app.exit(4);
                return;
            }
            qInfo("Saved screenshot: %s", qPrintable(outputPath));
            app.exit(0);
        });
    };

    QObject::connect(&view, &QQuickView::statusChanged,
                     &app, [&](QQuickView::Status status) {
        if (status == QQuickView::Error) {
            const auto errors = view.errors();
            for (const auto &error : errors)
                qCritical("%s", qPrintable(error.toString()));
            app.exit(5);
            return;
        }
        if (status == QQuickView::Ready)
            scheduleCapture();
    });

    view.setSource(source);
    if (view.status() == QQuickView::Error) {
        const auto errors = view.errors();
        for (const auto &error : errors)
            qCritical("%s", qPrintable(error.toString()));
        return 5;
    }
    scheduleCapture();

    return app.exec();
}
