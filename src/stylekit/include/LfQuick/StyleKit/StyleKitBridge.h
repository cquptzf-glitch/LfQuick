#pragma once

#include <LfQuick/StyleKit/LfQuickStyleKitGlobal.h>

#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKSTYLEKIT_EXPORT StyleKitBridge : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString stylePath READ stylePath WRITE setStylePath NOTIFY stylePathChanged FINAL)
    Q_PROPERTY(QString themeName READ themeName WRITE setThemeName NOTIFY themeNameChanged FINAL)
    Q_PROPERTY(bool applied READ isApplied NOTIFY appliedChanged FINAL)
    Q_PROPERTY(QStringList availableThemeNames READ availableThemeNames NOTIFY availableThemeNamesChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged FINAL)

public:
    explicit StyleKitBridge(QObject *parent = nullptr);

    QString stylePath() const { return m_stylePath; }
    void setStylePath(const QString &path);

    QString themeName() const { return m_themeName; }
    void setThemeName(const QString &name);

    bool isApplied() const;
    QStringList availableThemeNames() const;
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool applyToApplication();
    Q_INVOKABLE bool reload();

signals:
    void stylePathChanged();
    void themeNameChanged();
    void appliedChanged();
    void availableThemeNamesChanged();
    void lastErrorChanged();

private:
    void setLastError(const QString &error);
    bool validateStylePath(QString *error) const;

    QString m_stylePath;
    QString m_themeName = QStringLiteral("System");
    QString m_lastError;
    QPointer<QObject> m_styleObject;
};

} // namespace LfQuick
