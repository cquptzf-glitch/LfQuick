
#pragma once

#include <LfQuick/Core/LfQuickCoreGlobal.h>

#include <QObject>
#include <QString>

namespace LfQuick {

class LFQUICKCORE_EXPORT OperationHandle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged FINAL)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged FINAL)
    Q_PROPERTY(bool cancelable READ cancelable NOTIFY cancelableChanged FINAL)

public:
    enum class State {
        Idle,
        Running,
        Succeeded,
        Failed,
        Canceled
    };
    Q_ENUM(State)

    explicit OperationHandle(QObject *parent = nullptr);

    State state() const noexcept { return m_state; }
    double progress() const noexcept { return m_progress; }
    QString message() const { return m_message; }
    bool cancelable() const noexcept { return m_cancelable; }

    void setState(State state);
    void setProgress(double progress);
    void setMessage(QString message);
    void setCancelable(bool cancelable);

public slots:
    virtual void cancel();

signals:
    void stateChanged();
    void progressChanged();
    void messageChanged();
    void cancelableChanged();
    void cancelRequested();

private:
    State m_state = State::Idle;
    double m_progress = 0.0;
    QString m_message;
    bool m_cancelable = false;
};

} // namespace LfQuick
