#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QPointer>
#include <QObject>

namespace LfQuick {

class KeyboardWedgeScanner;

class LFQUICKPLATFORM_EXPORT InputRouter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KeyboardWedgeScanner* scanner READ scanner WRITE setScanner NOTIFY scannerChanged FINAL)
    Q_PROPERTY(bool installed READ installed NOTIFY installedChanged FINAL)

public:
    explicit InputRouter(QObject *parent = nullptr);
    ~InputRouter() override;

    KeyboardWedgeScanner *scanner() const;
    void setScanner(KeyboardWedgeScanner *scanner);

    bool installed() const;

    Q_INVOKABLE bool installOn(QObject *target);
    Q_INVOKABLE void uninstall();

signals:
    void scannerChanged();
    void installedChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPointer<KeyboardWedgeScanner> m_scanner;
    QPointer<QObject> m_target;
};

} // namespace LfQuick
