#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QQmlParserStatus>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class QmlStateSnapshot;

// Lifecycle hook fired after a QML object tree has completed state restoration.
//
// Unlike reacting to individual property change handlers, this hook runs only
// after QmlStateSnapshot has restored the complete keyed tree. Standalone QML
// objects also receive one queued initial-load completion after
// componentComplete(), matching Quickshell's PostReloadHook behavior outside a
// managed reload generation.
class LFQUICKUI_EXPORT PostReloadHook : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfPostReloadHook)
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(bool completed READ completed NOTIFY completionChanged FINAL)
    Q_PROPERTY(bool lastCompletionWasReload READ lastCompletionWasReload NOTIFY completionChanged FINAL)
    Q_PROPERTY(quint64 completionCount READ completionCount NOTIFY completionChanged FINAL)

public:
    explicit PostReloadHook(QObject *parent = nullptr);
    ~PostReloadHook() override = default;

    bool completed() const noexcept { return m_completed; }
    bool lastCompletionWasReload() const noexcept { return m_lastCompletionWasReload; }
    quint64 completionCount() const noexcept { return m_completionCount; }

    void classBegin() override;
    void componentComplete() override;

signals:
    // First-load completion with no previous serialized tree.
    void loaded();

    // Completion after replacing/recreating an older serialized tree.
    void reloaded();

    // Emitted for both paths after the whole restore phase is complete.
    void restoreCompleted(bool reloaded);
    void completionChanged();

protected:
    // C++ subclasses can perform work at the same deterministic lifecycle
    // boundary as the QML signals above.
    virtual void onPostReload(bool reloaded);

private:
    friend class QmlStateSnapshot;

    void completeRestore(bool reloaded);

    bool m_initialCompletionPending = false;
    bool m_completed = false;
    bool m_lastCompletionWasReload = false;
    quint64 m_completionCount = 0;
};

} // namespace LfQuick
