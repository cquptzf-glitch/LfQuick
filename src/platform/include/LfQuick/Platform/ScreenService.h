#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class QScreen;

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT ScreenService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY screensChanged FINAL)
    Q_PROPERTY(QVariantList screens READ screens NOTIFY screensChanged FINAL)
    Q_PROPERTY(QVariantMap primaryScreen READ primaryScreen NOTIFY screensChanged FINAL)

public:
    explicit ScreenService(QObject *parent = nullptr);

    int count() const;
    QVariantList screens() const;
    QVariantMap primaryScreen() const;

    Q_INVOKABLE QVariantMap screenAt(int x, int y) const;
    Q_INVOKABLE QVariantMap screenByName(const QString &name) const;

signals:
    void screensChanged();

private:
    void attachScreen(QScreen *screen);
    QVariantMap screenToMap(QScreen *screen) const;
};

} // namespace LfQuick
