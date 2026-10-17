#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QHash>
#include <QKeySequence>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT HotkeyRouter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool installed READ installed NOTIFY installedChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY hotkeysChanged FINAL)

public:
    explicit HotkeyRouter(QObject *parent = nullptr);
    ~HotkeyRouter() override;

    bool installed() const noexcept { return !m_target.isNull(); }
    int count() const noexcept { return m_hotkeys.size(); }

    Q_INVOKABLE bool installOn(QObject *target);
    Q_INVOKABLE void uninstall();
    Q_INVOKABLE bool registerHotkey(const QString &id,
                                    const QString &sequence,
                                    bool enabled = true,
                                    bool allowAutoRepeat = false);
    Q_INVOKABLE bool unregisterHotkey(const QString &id);
    Q_INVOKABLE bool setHotkeyEnabled(const QString &id, bool enabled);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantList hotkeys() const;

signals:
    void installedChanged();
    void hotkeysChanged();
    void triggered(const QString &id);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct Entry {
        QString id;
        QKeySequence sequence;
        bool enabled = true;
        bool allowAutoRepeat = false;
    };

    QHash<QString, Entry> m_hotkeys;
    QPointer<QObject> m_target;
};

} // namespace LfQuick
