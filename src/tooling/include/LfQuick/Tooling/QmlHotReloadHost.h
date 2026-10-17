#pragma once

#include <LfQuick/Tooling/LfQuickToolingGlobal.h>

#include <QPointer>
#include <QQuickItem>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class QmlHotReloadController;

class LFQUICKTOOLING_EXPORT QmlHotReloadHost : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QmlHotReloadController* controller READ controller WRITE setController NOTIFY controllerChanged FINAL)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QUrl developmentSource READ developmentSource WRITE setDevelopmentSource NOTIFY developmentSourceChanged FINAL)
    Q_PROPERTY(QVariantMap initialProperties READ initialProperties WRITE setInitialProperties NOTIFY initialPropertiesChanged FINAL)
    Q_PROPERTY(QStringList preservedProperties READ preservedProperties WRITE setPreservedProperties NOTIFY preservedPropertiesChanged FINAL)
    Q_PROPERTY(bool autoReload READ autoReload WRITE setAutoReload NOTIFY autoReloadChanged FINAL)
    Q_PROPERTY(QObject* item READ item NOTIFY itemChanged FINAL)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged FINAL)
    Q_PROPERTY(qint64 revision READ revision NOTIFY revisionChanged FINAL)

public:
    enum class Status {
        Idle,
        Reloading,
        Ready,
        Failed
    };
    Q_ENUM(Status)

    explicit QmlHotReloadHost(QQuickItem *parent = nullptr);
    ~QmlHotReloadHost() override;

    QmlHotReloadController *controller() const noexcept { return m_controller; }
    void setController(QmlHotReloadController *controller);

    QUrl source() const { return m_source; }
    void setSource(const QUrl &source);

    QUrl developmentSource() const { return m_developmentSource; }
    void setDevelopmentSource(const QUrl &source);

    QVariantMap initialProperties() const { return m_initialProperties; }
    void setInitialProperties(const QVariantMap &properties);

    QStringList preservedProperties() const { return m_preservedProperties; }
    void setPreservedProperties(const QStringList &properties);

    bool autoReload() const noexcept { return m_autoReload; }
    void setAutoReload(bool value);

    QObject *item() const noexcept { return m_item; }
    Status status() const noexcept { return m_status; }
    QString lastError() const { return m_lastError; }
    qint64 revision() const noexcept { return m_revision; }

    Q_INVOKABLE bool reload();

signals:
    void controllerChanged();
    void sourceChanged();
    void developmentSourceChanged();
    void initialPropertiesChanged();
    void preservedPropertiesChanged();
    void autoReloadChanged();
    void itemChanged();
    void statusChanged();
    void lastErrorChanged();
    void revisionChanged();
    void reloadSucceeded(qint64 revision);
    void reloadFailed(const QString &error, qint64 attemptedRevision);

protected:
    void componentComplete() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void registerWatch();
    bool reloadInternal(qint64 attemptedRevision);
    QVariantMap preservedState() const;
    void setStatus(Status status);
    void setLastError(const QString &error);
    QUrl effectiveSource() const;

    QPointer<QmlHotReloadController> m_controller;
    QUrl m_source;
    QUrl m_developmentSource;
    QVariantMap m_initialProperties;
    QStringList m_preservedProperties;
    bool m_autoReload = true;
    bool m_complete = false;
    QPointer<QQuickItem> m_item;
    Status m_status = Status::Idle;
    QString m_lastError;
    qint64 m_revision = 0;
};

} // namespace LfQuick
