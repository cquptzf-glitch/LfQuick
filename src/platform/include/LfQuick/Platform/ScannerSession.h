#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QElapsedTimer>
#include <QObject>

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT ScannerSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(int timeoutMs READ timeoutMs WRITE setTimeoutMs NOTIFY timeoutMsChanged FINAL)
    Q_PROPERTY(int minimumLength READ minimumLength WRITE setMinimumLength NOTIFY minimumLengthChanged FINAL)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY prefixChanged FINAL)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix NOTIFY suffixChanged FINAL)

public:
    explicit ScannerSession(QObject *parent = nullptr);

    bool isActive() const;
    int timeoutMs() const;
    void setTimeoutMs(int value);

    int minimumLength() const;
    void setMinimumLength(int value);

    QString prefix() const;
    void setPrefix(const QString &value);

    QString suffix() const;
    void setSuffix(const QString &value);

    Q_INVOKABLE void reset();
    Q_INVOKABLE void feed(const QString &text);
    Q_INVOKABLE bool finish();
    Q_INVOKABLE void simulateScan(const QString &value);

signals:
    void activeChanged();
    void timeoutMsChanged();
    void minimumLengthChanged();
    void prefixChanged();
    void suffixChanged();

    void scanAccepted(const QString &value);
    void scanRejected(const QString &rawValue, const QString &reason);

private:
    bool validateAndEmit();

    bool m_active = false;
    int m_timeoutMs = 45;
    int m_minimumLength = 4;
    QString m_prefix;
    QString m_suffix;
    QString m_buffer;
    QElapsedTimer m_timer;
};

} // namespace LfQuick
