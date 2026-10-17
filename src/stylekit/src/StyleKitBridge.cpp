#include <LfQuick/StyleKit/StyleKitBridge.h>

#include <QApplication>
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
    return m_styleObject && QApplication::instance()
        && QApplication::style() == m_styleObject.data();
}

QStringList StyleKitBridge::availableThemeNames() const
{
    if (const auto *style = qobject_cast<const QStyleKitStyle *>(m_styleObject.data()))
        return style->availableThemeNames();
    return {};
}

bool StyleKitBridge::applyToApplication()
{
    if (!QApplication::instance()) {
        setLastError(QStringLiteral("StyleKitBridge requires QApplication"));
        return false;
    }

    QString validationError;
    if (!validateStylePath(&validationError)) {
        setLastError(validationError);
        return false;
    }

    auto *candidate = new QStyleKitStyle(m_stylePath);
    const QStringList themes = candidate->availableThemeNames();
    if (themes.isEmpty()) {
        delete candidate;
        setLastError(QStringLiteral("StyleKit style could not be loaded: %1").arg(m_stylePath));
        return false;
    }

    if (m_themeName != QStringLiteral("System") && !themes.contains(m_themeName)) {
        delete candidate;
        setLastError(QStringLiteral("StyleKit theme '%1' is not available in %2")
                         .arg(m_themeName, m_stylePath));
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

    const bool wasApplied = isApplied();
    QApplication::setStyle(candidate);
    m_styleObject = candidate;
    setLastError({});

    if (!wasApplied || isApplied())
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
