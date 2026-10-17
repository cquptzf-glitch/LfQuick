#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QObject>
#include <QString>
#include <QVariantMap>

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT IInputSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT FINAL)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)

public:
    explicit IInputSource(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~IInputSource() override = default;

    virtual QString id() const = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

signals:
    void enabledChanged();
    void inputReceived(const QString &value, const QVariantMap &metadata);
    void inputRejected(const QString &reason, const QVariantMap &metadata);
};

} // namespace LfQuick
