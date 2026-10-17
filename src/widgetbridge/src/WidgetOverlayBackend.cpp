#include <LfQuick/WidgetBridge/WidgetOverlayBackend.h>

#include <QApplication>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

namespace LfQuick {

WidgetOverlayBackend::WidgetOverlayBackend(QWidget *parentWidget, QObject *parent)
    : IOverlayBackend(parent)
{
    setParentWidget(parentWidget);
}

WidgetOverlayBackend::~WidgetOverlayBackend()
{
    if (m_parentWidget)
        m_parentWidget->removeEventFilter(this);
}

void WidgetOverlayBackend::setParentWidget(QWidget *widget)
{
    if (m_parentWidget == widget)
        return;

    if (m_parentWidget)
        m_parentWidget->removeEventFilter(this);
    m_parentWidget = widget;
    if (m_parentWidget)
        m_parentWidget->installEventFilter(this);

    for (auto it = m_toasts.begin(); it != m_toasts.end(); ++it) {
        if (!it->frame)
            continue;
        it->frame->setParent(m_parentWidget);
        it->frame->show();
        it->frame->raise();
    }
    layoutToasts();
}

void WidgetOverlayBackend::presentToast(const ToastRequest &request)
{
    if (request.id.trimmed().isEmpty() || request.message.trimmed().isEmpty())
        return;

    dismissToast(request.id);
    QWidget *parent = effectiveParentWidget();
    if (!parent)
        return;

    auto *frame = new QFrame(parent);
    frame->setObjectName(QStringLiteral("lfquickToast"));
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setAutoFillBackground(true);
    frame->setMinimumWidth(260);
    frame->setMaximumWidth(420);

    auto *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(12, 10, 8, 10);
    layout->setSpacing(8);

    QString severityText;
    if (request.severity == QStringLiteral("success"))
        severityText = tr("Success");
    else if (request.severity == QStringLiteral("warning"))
        severityText = tr("Warning");
    else if (request.severity == QStringLiteral("error"))
        severityText = tr("Error");
    else
        severityText = tr("Info");

    auto *severity = new QLabel(severityText, frame);
    QFont severityFont = severity->font();
    severityFont.setBold(true);
    severity->setFont(severityFont);
    layout->addWidget(severity, 0, Qt::AlignTop);

    auto *message = new QLabel(request.message, frame);
    message->setWordWrap(true);
    message->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(message, 1);

    auto *closeButton = new QToolButton(frame);
    closeButton->setText(QStringLiteral("×"));
    closeButton->setToolTip(tr("Dismiss notification"));
    layout->addWidget(closeButton, 0, Qt::AlignTop);

    connect(closeButton, &QToolButton::clicked, this, [this, id = request.id] {
        emit toastDismissRequested(id);
    });

    ToastWidget widget;
    widget.request = request;
    widget.frame = frame;
    m_toasts.insert(request.id, widget);
    m_toastOrder.append(request.id);

    connect(frame, &QObject::destroyed, this, [this, id = request.id] {
        auto it = m_toasts.find(id);
        // A toast with the same id may already have been replaced while the
        // previous frame was waiting for deferred deletion. Only reconcile
        // the entry whose guarded frame was actually destroyed.
        if (it == m_toasts.end() || !it->frame.isNull())
            return;
        m_toasts.erase(it);
        m_toastOrder.removeAll(id);
        layoutToasts();

        // Native parent destruction can recursively destroy children. Defer
        // the logical dismissal until that native destructor stack is done.
        QMetaObject::invokeMethod(this, [this, id] {
            emit toastDismissRequested(id);
        }, Qt::QueuedConnection);
    });

    frame->adjustSize();
    frame->show();
    frame->raise();
    layoutToasts();
}

void WidgetOverlayBackend::dismissToast(const QString &requestId)
{
    auto it = m_toasts.find(requestId);
    if (it == m_toasts.end())
        return;

    if (it->frame)
        it->frame->deleteLater();
    m_toasts.erase(it);
    m_toastOrder.removeAll(requestId);
    layoutToasts();
}

void WidgetOverlayBackend::presentDialog(const DialogRequest &request)
{
    if (request.id.trimmed().isEmpty())
        return;

    dismissDialog(request.id);
    QWidget *parent = effectiveParentWidget();
    auto *box = new QMessageBox(parent);
    box->setAttribute(Qt::WA_DeleteOnClose, false);
    box->setWindowTitle(request.title);
    box->setText(request.message);
    box->setTextFormat(Qt::PlainText);
    box->setIcon(request.buttons.contains(QStringLiteral("yes"))
                     || request.buttons.contains(QStringLiteral("no"))
                 ? QMessageBox::Question
                 : QMessageBox::Information);
    box->setWindowModality(parent ? Qt::WindowModal : Qt::ApplicationModal);

    DialogWidget widget;
    widget.box = box;
    widget.fallbackButton = request.defaultButton;
    if (request.buttons.contains(QStringLiteral("cancel")))
        widget.fallbackButton = QStringLiteral("cancel");
    else if (request.buttons.contains(QStringLiteral("close")))
        widget.fallbackButton = QStringLiteral("close");
    else if (request.buttons.contains(QStringLiteral("no")))
        widget.fallbackButton = QStringLiteral("no");

    QPushButton *defaultButton = nullptr;
    for (const QString &buttonId : request.buttons) {
        auto *button = box->addButton(buttonText(buttonId),
                                      buttonRole(buttonId, request.destructiveButton));
        widget.buttonIds.insert(button, buttonId);
        if (buttonId == request.defaultButton)
            defaultButton = button;
    }
    if (defaultButton)
        box->setDefaultButton(defaultButton);

    m_dialogs.insert(request.id, widget);
    auto it = m_dialogs.find(request.id);
    it->finishedConnection = connect(box, &QMessageBox::finished, this,
                                     [this, requestId = request.id] {
        auto dialogIt = m_dialogs.find(requestId);
        if (dialogIt == m_dialogs.end())
            return;

        QString buttonId = dialogIt->fallbackButton;
        if (dialogIt->box && dialogIt->box->clickedButton())
            buttonId = dialogIt->buttonIds.value(dialogIt->box->clickedButton(), buttonId);

        QObject::disconnect(dialogIt->finishedConnection);
        if (dialogIt->box)
            dialogIt->box->deleteLater();
        m_dialogs.erase(dialogIt);
        emit dialogResolved(requestId, buttonId);
    });

    connect(box, &QObject::destroyed, this, [this, requestId = request.id] {
        auto dialogIt = m_dialogs.find(requestId);
        // Normal resolve/dismiss erases the entry before deferred deletion.
        // Reaching this branch means the native dialog disappeared externally,
        // usually because its parent window was destroyed.
        if (dialogIt == m_dialogs.end() || !dialogIt->box.isNull())
            return;
        const QString fallback = dialogIt->fallbackButton;
        QObject::disconnect(dialogIt->finishedConnection);
        m_dialogs.erase(dialogIt);

        QMetaObject::invokeMethod(this, [this, requestId, fallback] {
            emit dialogResolved(requestId, fallback);
        }, Qt::QueuedConnection);
    });

    box->open();
}

void WidgetOverlayBackend::dismissDialog(const QString &requestId)
{
    auto it = m_dialogs.find(requestId);
    if (it == m_dialogs.end())
        return;

    QObject::disconnect(it->finishedConnection);
    if (it->box) {
        it->box->close();
        it->box->deleteLater();
    }
    m_dialogs.erase(it);
}

bool WidgetOverlayBackend::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_parentWidget
        && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
        layoutToasts();
    }
    return IOverlayBackend::eventFilter(watched, event);
}

QWidget *WidgetOverlayBackend::effectiveParentWidget() const
{
    if (m_parentWidget)
        return m_parentWidget;
    return QApplication::activeWindow();
}

void WidgetOverlayBackend::layoutToasts()
{
    QWidget *parent = effectiveParentWidget();
    if (!parent)
        return;

    QHash<QString, int> offsets;
    constexpr int margin = 16;
    constexpr int spacing = 8;

    for (const QString &id : m_toastOrder) {
        auto it = m_toasts.find(id);
        if (it == m_toasts.end() || !it->frame)
            continue;

        QFrame *frame = it->frame;
        frame->adjustSize();
        const QString position = it->request.position;
        const bool bottom = position.startsWith(QStringLiteral("bottom-"));
        const bool center = position.endsWith(QStringLiteral("-center"));
        const int offset = offsets.value(position, 0);

        const int x = center
            ? (parent->width() - frame->width()) / 2
            : parent->width() - margin - frame->width();
        const int y = bottom
            ? parent->height() - margin - frame->height() - offset
            : margin + offset;

        frame->move(qMax(margin, x), qMax(margin, y));
        frame->raise();
        offsets.insert(position, offset + frame->height() + spacing);
    }
}

QString WidgetOverlayBackend::buttonText(const QString &buttonId)
{
    if (buttonId == QStringLiteral("ok"))
        return tr("OK");
    if (buttonId == QStringLiteral("cancel"))
        return tr("Cancel");
    if (buttonId == QStringLiteral("yes"))
        return tr("Yes");
    if (buttonId == QStringLiteral("no"))
        return tr("No");
    if (buttonId == QStringLiteral("retry"))
        return tr("Retry");
    if (buttonId == QStringLiteral("close"))
        return tr("Close");
    if (buttonId == QStringLiteral("discard"))
        return tr("Discard");
    if (buttonId == QStringLiteral("apply"))
        return tr("Apply");
    return buttonId;
}

QMessageBox::ButtonRole WidgetOverlayBackend::buttonRole(
    const QString &buttonId, const QString &destructiveButton)
{
    if (buttonId == destructiveButton || buttonId == QStringLiteral("discard"))
        return QMessageBox::DestructiveRole;
    if (buttonId == QStringLiteral("cancel") || buttonId == QStringLiteral("close"))
        return QMessageBox::RejectRole;
    if (buttonId == QStringLiteral("yes"))
        return QMessageBox::YesRole;
    if (buttonId == QStringLiteral("no"))
        return QMessageBox::NoRole;
    if (buttonId == QStringLiteral("retry"))
        return QMessageBox::ActionRole;
    if (buttonId == QStringLiteral("apply"))
        return QMessageBox::ApplyRole;
    return QMessageBox::AcceptRole;
}

} // namespace LfQuick
