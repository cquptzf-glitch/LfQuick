#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QHash>
#include <QList>
#include <QMetaProperty>
#include <QPointer>
#include <QQuickItem>
#include <QUrl>
#include <QQmlComponent>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class QmlObjectFactory;
class QmlObjectLoadHandle;

// Dynamic component host that forwards QML-declared properties and signal
// handlers to its loaded object. Custom properties declared on an
// LfBoundComponent instance are passed as initial properties (including
// required properties) and, when bindValues=true, remain reactive afterwards.
class LFQUICKUI_EXPORT BoundComponent final : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfBoundComponent)
    Q_CLASSINFO("DefaultProperty", "component")

    Q_PROPERTY(QObject* item READ item NOTIFY itemChanged FINAL)
    Q_PROPERTY(QQmlComponent* component READ component WRITE setComponent NOTIFY componentChanged FINAL)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(bool bindValues READ bindValues WRITE setBindValues NOTIFY bindValuesChanged FINAL)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged FINAL)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)

public:
    explicit BoundComponent(QQuickItem *parent = nullptr);
    ~BoundComponent() override;

    QObject *item() const noexcept { return m_item; }

    QQmlComponent *component() const noexcept { return m_component; }
    void setComponent(QQmlComponent *component);

    QUrl source() const { return m_source; }
    void setSource(const QUrl &source);

    bool bindValues() const noexcept { return m_bindValues; }
    void setBindValues(bool bindValues);

    bool asynchronous() const noexcept { return m_asynchronous; }
    void setAsynchronous(bool asynchronous);

    bool loading() const noexcept;
    QString errorString() const { return m_errorString; }

    Q_INVOKABLE void reload();
    Q_INVOKABLE void clear();

signals:
    void itemChanged();
    void componentChanged();
    void sourceChanged();
    void bindValuesChanged();
    void asynchronousChanged();
    void loadingChanged();
    void errorStringChanged();
    void loaded();
    void failed(const QString &message);

protected:
    void componentComplete() override;

private slots:
    void forwardDynamicProperty();

private:
    struct PropertyBinding {
        QMetaProperty source;
        QMetaProperty target;
    };

    QVariantMap collectInitialProperties() const;
    void startLoad();
    void finishHandle();
    void adoptObject(QObject *object);
    void bindDynamicProperties();
    void bindSignalHandlers();
    void updateItemSize();
    void updateImplicitSize();
    void unloadObject();
    void cancelHandle();
    void reconnectComponent();
    void clearBindings();
    QUrl resolvedSource() const;
    void setErrorString(const QString &error);

    QmlObjectFactory *m_factory = nullptr;
    QPointer<QmlObjectLoadHandle> m_handle;
    QPointer<QObject> m_item;
    QPointer<QQuickItem> m_quickItem;
    QPointer<QQmlComponent> m_component;
    QMetaObject::Connection m_componentDestroyedConnection;
    QList<QMetaObject::Connection> m_bindingConnections;
    QList<QMetaObject::Connection> m_itemConnections;
    QHash<int, QList<PropertyBinding>> m_propertyBindings;
    QUrl m_source;
    QString m_errorString;
    bool m_bindValues = true;
    bool m_asynchronous = true;
    bool m_complete = false;
};

} // namespace LfQuick
