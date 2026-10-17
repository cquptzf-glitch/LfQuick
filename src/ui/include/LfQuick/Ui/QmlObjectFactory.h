#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariantMap>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

class QQmlIncubator;

namespace LfQuick {

class QmlObjectFactory;

class LFQUICKUI_EXPORT QmlObjectLoadHandle : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QUrl source READ source CONSTANT FINAL)
    Q_PROPERTY(QQmlComponent* component READ component CONSTANT FINAL)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(QObject* object READ object NOTIFY objectChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)

public:
    enum class Status {
        Loading,
        Ready,
        Error,
        Canceled
    };
    Q_ENUM(Status)

    ~QmlObjectLoadHandle() override;

    QUrl source() const { return m_source; }
    QQmlComponent *component() const noexcept { return m_component; }
    Status status() const noexcept { return m_status; }
    QObject *object() const noexcept { return m_object; }
    QString errorString() const { return m_errorString; }

    Q_INVOKABLE void cancel();
    Q_INVOKABLE void forceCompletion();
    Q_INVOKABLE QObject *takeObject();

signals:
    void statusChanged();
    void objectChanged();
    void errorStringChanged();
    void finished();

private:
    friend class QmlObjectFactory;
    friend class FactoryIncubator;

    explicit QmlObjectLoadHandle(QUrl source,
                                 QVariantMap initialProperties,
                                 QObject *parent = nullptr);
    explicit QmlObjectLoadHandle(QQmlComponent *component,
                                 QVariantMap initialProperties,
                                 QObject *parent = nullptr);

    void begin(QQmlEngine *engine);
    void startIncubation();
    void onComponentStatusChanged();
    void onIncubatorStatusChanged();
    void finishWithObject(QObject *object);
    void fail(const QString &message);
    void setStatus(Status status);
    void setErrorString(const QString &message);

    QUrl m_source;
    QVariantMap m_initialProperties;
    QPointer<QQmlEngine> m_engine;
    std::unique_ptr<QQmlComponent> m_ownedComponent;
    QPointer<QQmlComponent> m_component;
    QMetaObject::Connection m_componentDestroyedConnection;
    std::unique_ptr<QQmlIncubator> m_incubator;
    QPointer<QObject> m_object;
    Status m_status = Status::Loading;
    QString m_errorString;
};

class LFQUICKUI_EXPORT QmlObjectFactory : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfObjectFactory)
    Q_PROPERTY(QQmlEngine* engine READ engine WRITE setEngine NOTIFY engineChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged FINAL)

public:
    explicit QmlObjectFactory(QObject *parent = nullptr);

    QQmlEngine *engine() const noexcept { return m_engine; }
    void setEngine(QQmlEngine *engine);

    QString lastError() const { return m_lastError; }

    Q_INVOKABLE QObject *create(const QUrl &source,
                                const QVariantMap &initialProperties = {});
    Q_INVOKABLE QObject *createComponent(QQmlComponent *component,
                                         const QVariantMap &initialProperties = {});
    Q_INVOKABLE QmlObjectLoadHandle *createAsync(const QUrl &source,
                                                  const QVariantMap &initialProperties = {});
    Q_INVOKABLE QmlObjectLoadHandle *createAsyncComponent(
        QQmlComponent *component,
        const QVariantMap &initialProperties = {});

    static QObject *createSync(QQmlEngine *engine,
                               const QUrl &source,
                               const QVariantMap &initialProperties,
                               QString *error = nullptr);
    static QObject *createSync(QQmlComponent *component,
                               const QVariantMap &initialProperties,
                               QString *error = nullptr);

signals:
    void engineChanged();
    void lastErrorChanged();

private:
    QmlObjectLoadHandle *beginAsync(QmlObjectLoadHandle *handle, QQmlEngine *engine);
    QQmlEngine *effectiveEngine() const;
    void setLastError(const QString &message);

    QPointer<QQmlEngine> m_engine;
    QString m_lastError;
};

} // namespace LfQuick
