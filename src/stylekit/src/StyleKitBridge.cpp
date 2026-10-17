#include <LfQuick/StyleKit/StyleKitBridge.h>

#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QStyle>
#include <QStyleKitStyle>

namespace LfQuick {

StyleKitBridge::StyleKitBridge(QObject *parent)
    : QObject(parent)
{
}

void StyleKitBridge::setStylePath(const QString &path)
{
    if (m_stylePath == path)
        return;
    m_stylePath = path;
    emit stylePathChanged();
}

void StyleKitBridge::setThemeName(const QString &name)
{
    const QString normalized = name.trimmed().isEmpty()
        ? QStringLiteral("System")
        : name.trimmed();

    if (m_themeName == normalized)
        return;

    if (auto *style = qobject_cast<QStyleKitStyle *>(m_styleObject.data())) {
        const QStringList names = style->availableThemeNames();
        if (normalized != QStringLiteral("System") && !names.contains(normalized)) {
            setLastError(QStringLiteral("StyleKit theme '%1' is not available").arg(normalized));
            return;
        }
        style->setThemeName(normalized);
    }

    m_themeName = normalized;
    setLastError({});
    emit themeNameChanged();
}

bool StyleKitBridge::isApplied() const
{
    auto *application = qobject_cast<QApplication *>(QCoreApplication::instance());
    auto *style = qobject_cast<QStyle *>(m_styleObject.data());
    return application && style && QApplication::style() == style;
}

QStringList StyleKitBridge::availableThemeNames() const
{
    if (const auto *style = qobject_cast<const QStyleKitStyle *>(m_styleObject.data()))
        return style->availableThemeNames();
    return {};
}

bool StyleKitBridge::applyToApplication()
{
    if (!qobject_cast<QApplication *>(QCoreApplication::instance())) {
        setLastError(QStringLiteral("StyleKitBridge requires QApplication"));
        return false;
    }

    QString validationError;
    if (!validateStylePath(&validationError)) {
        setLastError(validationError);
        return false;
    }

    const QString requestedPath = m_stylePath.trimmed();
    auto *candidate = new QStyleKitStyle;
    const QString previousCandidatePath = candidate->stylePath();
    candidate->setStylePath(requestedPath);

    // QStyleKitStyle keeps the previously loaded style when setStylePath()
    // fails. A fresh candidate therefore retains its initial empty path on a
    // syntax/root-type/load failure instead of silently accepting fallback
    // rendering as a valid application style.
    if (candidate->stylePath() == previousCandidatePath
        || candidate->stylePath() != requestedPath) {
        delete candidate;
        setLastError(QStringLiteral("StyleKit style could not be loaded as a Style root: %1")
                         .arg(requestedPath));
        return false;
    }

    const QStringList themes = candidate->availableThemeNames();
    if (themes.isEmpty()) {
        delete candidate;
        setLastError(QStringLiteral("StyleKit style exposes no themes: %1").arg(requestedPath));
        return false;
    }

    if (m_themeName != QStringLiteral("System") && !themes.contains(m_themeName)) {
        delete candidate;
        setLastError(QStringLiteral("StyleKit theme '%1' is not available in %2")
                         .arg(m_themeName, requestedPath));
        return false;
    }

    candidate->setThemeName(m_themeName);

    connect(candidate, &QStyleKitStyle::availableThemeNamesChanged,
            this, [this](const QStringList &) {
                emit availableThemeNamesChanged();
            });
    connect(candidate, &QStyleKitStyle::themeNameChanged,
            this, [this](const QString &themeName) {
                if (m_themeName == themeName)
                    return;
                m_themeName = themeName;
                emit themeNameChanged();
            });

    QApplication::setStyle(candidate);
    m_styleObject = candidate;
    setLastError({});

    emit appliedChanged();
    emit availableThemeNamesChanged();
    return true;
}

bool StyleKitBridge::reload()
{
    return applyToApplication();
}

void StyleKitBridge::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

bool StyleKitBridge::validateStylePath(QString *error) const
{
    if (m_stylePath.trimmed().isEmpty()) {
        if (error)
            *error = QStringLiteral("StyleKit style path is empty");
        return false;
    }

    const QString path = m_stylePath.trimmed();
    if ((path.startsWith(QStringLiteral(":/")) || path.startsWith(QStringLiteral("qrc:/")))) {
        const QString resourcePath = path.startsWith(QStringLiteral("qrc:/"))
            ? QStringLiteral(":/") + path.mid(5)
            : path;
        if (!QFile::exists(resourcePath)) {
            if (error)
                *error = QStringLiteral("StyleKit resource does not exist: %1").arg(path);
            return false;
        }
        return true;
    }

    if (!QFile::exists(path)) {
        if (error)
            *error = QStringLiteral("StyleKit file does not exist: %1").arg(path);
        return false;
    }
    return true;
}

} // namespace LfQuick
