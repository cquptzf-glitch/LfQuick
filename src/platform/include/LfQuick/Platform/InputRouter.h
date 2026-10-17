#pragma once

#include <LfQuick/Platform/IInputSource.h>
#include <LfQuick/Platform/InputTarget.h>
#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QHash>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QVariantMap>

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT InputRouter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KeyboardWedgeScanner* scanner READ scanner WRITE setScanner NOTIFY scannerChanged FINAL)
    Q_PROPERTY(bool installed READ installed NOTIFY installedChanged FINAL)
    Q_PROPERTY(int sourceCount READ sourceCount NOTIFY sourcesChanged FINAL)
    Q_PROPERTY(int targetCount READ targetCount NOTIFY targetsChanged FINAL)

public:
    explicit InputRouter(QObject *parent = nullptr);
    ~InputRouter() override;

    KeyboardWedgeScanner *scanner() const;
    void setScanner(KeyboardWedgeScanner *scanner);

    bool installed() const;
    int sourceCount() const noexcept { return m_sources.size(); }
    int targetCount() const noexcept;

    bool registerSource(IInputSource *source);
    bool unregisterSource(IInputSource *source);
    void clearSources();

    Q_INVOKABLE bool registerTarget(QObject *target);
    Q_INVOKABLE bool unregisterTarget(QObject *target);
    Q_INVOKABLE void clearTargets();

    Q_INVOKABLE bool installOn(QObject *target);
    Q_INVOKABLE void uninstall();

signals:
    void scannerChanged();
    void installedChanged();
    void sourcesChanged();
    void targetsChanged();
    void inputRouted(const QString &targetId,
                     const QString &sourceId,
                     const QVariantMap &metadata);
    void inputUnhandled(const QString &value, const QVariantMap &metadata);
    void sourceRejected(const QString &sourceId,
                        const QString &reason,
                        const QVariantMap &metadata);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct SourceConnections {
        QMetaObject::Connection received;
        QMetaObject::Connection rejected;
        QMetaObject::Connection destroyed;
    };

    void routeInput(const QString &value, const QVariantMap &metadata);
    void pruneTargets();

    QPointer<KeyboardWedgeScanner> m_scanner;
    QPointer<QObject> m_target;
    QHash<IInputSource *, SourceConnections> m_sources;
    QList<QPointer<InputTarget>> m_targets;
};

} // namespace LfQuick
