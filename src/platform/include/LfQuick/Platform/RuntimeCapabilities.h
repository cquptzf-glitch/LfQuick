#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QObject>
#include <QString>
#include <QVariantMap>

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT RuntimeCapabilities : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT FINAL)
    Q_PROPERTY(QString platformName READ platformName CONSTANT FINAL)
    Q_PROPERTY(bool debugBuild READ debugBuild CONSTANT FINAL)
    Q_PROPERTY(QVariantMap capabilities READ capabilities CONSTANT FINAL)

public:
    explicit RuntimeCapabilities(QObject *parent = nullptr);

    QString qtVersion() const;
    QString platformName() const;
    bool debugBuild() const noexcept;
    QVariantMap capabilities() const;

    Q_INVOKABLE bool hasCapability(const QString &id) const;
};

} // namespace LfQuick
