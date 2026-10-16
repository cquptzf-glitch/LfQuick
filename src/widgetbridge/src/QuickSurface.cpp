
#include <LfQuick/WidgetBridge/QuickSurface.h>

#include <QColor>

namespace LfQuick {

QuickSurface::QuickSurface(QWidget *parent)
    : QQuickWidget(parent)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setClearColor(Qt::transparent);
    setFocusPolicy(Qt::StrongFocus);
}

} // namespace LfQuick
