#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>
#include <LfQuick/Ui/NavigationItemModel.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT NavigationController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(NavigationItemModel* model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(QString currentRoute READ currentRoute NOTIFY navigationStateChanged FINAL)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationStateChanged FINAL)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationStateChanged FINAL)

public:
    explicit NavigationController(QObject *parent = nullptr);

    NavigationItemModel *model() const noexcept { return m_model; }
    void setModel(NavigationItemModel *model);

    QString currentRoute() const { return m_currentRoute; }
    bool canGoBack() const noexcept { return m_canGoBack; }
    bool canGoForward() const noexcept { return m_canGoForward; }

    Q_INVOKABLE bool navigate(const QString &route, const QVariantMap &parameters = {});
    Q_INVOKABLE bool back();
    Q_INVOKABLE bool forward();
    Q_INVOKABLE void syncCurrentRoute(const QString &route, bool canGoBack);
    Q_INVOKABLE void syncNavigationState(const QString &route,
                                         bool canGoBack,
                                         bool canGoForward);

signals:
    void modelChanged();
    void navigationStateChanged();
    void navigateRequested(const QString &route, const QVariantMap &parameters);
    void backRequested();
    void forwardRequested();
    void navigationRejected(const QString &route, const QString &reason);

private:
    QPointer<NavigationItemModel> m_model;
    QString m_currentRoute;
    bool m_canGoBack = false;
    bool m_canGoForward = false;
};

} // namespace LfQuick
