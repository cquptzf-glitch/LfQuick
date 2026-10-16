
#pragma once

#include <QQuickWidget>

namespace LfQuick {

class QuickSurface : public QQuickWidget
{
    Q_OBJECT
public:
    explicit QuickSurface(QWidget *parent = nullptr);
};

} // namespace LfQuick
