#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

// A small QML state container whose QML-declared properties can be restored
// across hot reloads or instance recreation.
class LFQUICKUI_EXPORT PersistentState : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged FINAL)

public:
    explicit PersistentState(QObject *parent = nullptr);

    QString key() const { return m_key; }
    void setKey(const QString &key);

    QVariantMap saveState() const;

    // Restores a previously serialized state. loaded() is always emitted;
    // reloaded() is emitted when the state originated from an older instance.
    int restoreState(const QVariantMap &state, bool reloaded = true);
    void markLoaded();

    // Compatibility/convenience path for direct old/new object transfer.
    int restoreFrom(QObject *oldInstance);

signals:
    void keyChanged();
    void loaded();
    void reloaded();

private:
    QString m_key;
};

} // namespace LfQuick
