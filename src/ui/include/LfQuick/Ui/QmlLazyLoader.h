#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariantMap>
#include <QQmlParserStatus>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class QmlObjectFactory;
class QmlObjectLoadHandle;

class LFQUICKUI_EXPORT QmlLazyLoader : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfLazyLoader)
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QObject* item READ item NOTIFY itemChanged FINAL)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged FINAL)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(bool activeAsync READ isActive WRITE setActiveAsync NOTIFY activeChanged FINAL)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QVariantMap initialProperties READ initialProperties WRITE setInitialProperties NOTIFY initialPropertiesChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)

public:
    explicit QmlLazyLoader(QObject *parent = nullptr);
    ~QmlLazyLoader() override;

    QObject *item();

    bool isLoading() const noexcept;
    void setLoading(bool loading);

    bool isActive() const noexcept { return !m_item.isNull(); }
    void setActive(bool active);
    void setActiveAsync(bool active);

    QUrl source() const { return m_source; }
    void setSource(const QUrl &source);

    QVariantMap initialProperties() const { return m_initialProperties; }
    void setInitialProperties(const QVariantMap &properties);

    QString errorString() const { return m_errorString; }

    void classBegin() override;
    void componentComplete() override;

signals:
    void itemChanged();
    void loadingChanged();
    void activeChanged();
    void sourceChanged();
    void initialPropertiesChanged();
    void errorStringChanged();

private:
    void startAsync();
    void startSync();
    void finishHandle();
    void cancelHandle();
    void adoptItem(QObject *item);
    void unloadItem();
    void setErrorString(const QString &error);
    QUrl resolvedSource() const;

    QmlObjectFactory *m_factory = nullptr;
    QPointer<QmlObjectLoadHandle> m_handle;
    QPointer<QObject> m_item;
    QUrl m_source;
    QVariantMap m_initialProperties;
    QString m_errorString;
    bool m_complete = false;
    bool m_targetLoading = false;
    bool m_targetActive = false;
};

} // namespace LfQuick
