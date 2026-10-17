#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

// A small QML state container whose QML-declared properties can be restored
// across hot reloads. Its behavior is intentionally aligned with Quickshell's
// PersistentProperties while using LfQuick's scoped reload runtime.
class LFQUICKUI_EXPORT PersistentState : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged FINAL)

public:
    explicit PersistentState(QObject *parent = nullptr);

    QString key() const { return m_key; }
    void setKey(const QString &key);

    // Restores properties declared by the concrete QML type. Returns the
    // number of properties successfully restored. Passing nullptr only marks
    // the initial load as complete.
    int restoreFrom(QObject *oldInstance);

signals:
    void keyChanged();
    void loaded();
    void reloaded();

private:
    QString m_key;
};

} // namespace LfQuick
