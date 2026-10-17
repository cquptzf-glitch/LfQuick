#include <LfQuick/Ui/PopupAnchor.h>

#include <QPointer>
#include <QQuickItem>
#include <QSignalSpy>
#include <QTest>

using namespace LfQuick;

class PopupAnchorTest : public QObject
{
    Q_OBJECT

private slots:
    void rotatedAnchorUsesFullMappedBounds()
    {
        QQuickItem root;
        root.setWidth(500.0);
        root.setHeight(500.0);

        QQuickItem anchor(&root);
        anchor.setX(100.0);
        anchor.setY(100.0);
        anchor.setWidth(100.0);
        anchor.setHeight(20.0);
        anchor.setTransformOrigin(QQuickItem::TopLeft);
        anchor.setRotation(45.0);

        PopupAnchor popup;
        popup.setAnchorItem(&anchor);
        popup.setParentItem(&root);
        popup.setEdge(PopupAnchor::Edge::Bottom);
        popup.setAlignment(PopupAnchor::Alignment::Start);
        popup.setFlip(false);
        popup.setMargin(0.0);
        popup.setPopupSize(QSizeF(10.0, 10.0));

        const QRectF mapped = anchor.mapRectToItem(
            &root, QRectF(0.0, 0.0, anchor.width(), anchor.height()));
        const QPointF expected(mapped.left(), mapped.bottom());

        QVERIFY(qAbs(popup.position().x() - expected.x()) < 0.001);
        QVERIFY(qAbs(popup.position().y() - expected.y()) < 0.001);
    }

    void ancestorTransformAutomaticallyRefreshesPosition()
    {
        QQuickItem root;
        root.setWidth(600.0);
        root.setHeight(600.0);

        QQuickItem container(&root);
        container.setX(50.0);
        container.setY(40.0);
        container.setWidth(300.0);
        container.setHeight(300.0);
        container.setTransformOrigin(QQuickItem::TopLeft);

        QQuickItem anchor(&container);
        anchor.setX(30.0);
        anchor.setY(20.0);
        anchor.setWidth(80.0);
        anchor.setHeight(20.0);

        PopupAnchor popup;
        popup.setAnchorItem(&anchor);
        popup.setParentItem(&root);
        popup.setEdge(PopupAnchor::Edge::Bottom);
        popup.setFlip(false);
        popup.setMargin(0.0);
        popup.setPopupSize(QSizeF(20.0, 10.0));

        const QPointF before = popup.position();
        QSignalSpy changed(&popup, &PopupAnchor::positionChanged);

        container.setRotation(30.0);

        const QRectF mapped = anchor.mapRectToItem(
            &root, QRectF(0.0, 0.0, anchor.width(), anchor.height()));
        const QPointF expected(mapped.left(), mapped.bottom());
        QVERIFY(popup.position() != before);
        QVERIFY(qAbs(popup.position().x() - expected.x()) < 0.001);
        QVERIFY(qAbs(popup.position().y() - expected.y()) < 0.001);
        QVERIFY(changed.count() >= 1);
    }

    void endpointDestructionClearsObservableProperties()
    {
        auto *root = new QQuickItem;
        root->setWidth(300.0);
        root->setHeight(300.0);
        auto *anchor = new QQuickItem(root);
        anchor->setWidth(20.0);
        anchor->setHeight(20.0);

        PopupAnchor popup;
        popup.setParentItem(root);
        popup.setAnchorItem(anchor);

        QSignalSpy anchorChanged(&popup, &PopupAnchor::anchorItemChanged);
        delete anchor;
        QCOMPARE(popup.anchorItem(), nullptr);
        QCOMPARE(anchorChanged.count(), 1);

        QSignalSpy parentChanged(&popup, &PopupAnchor::parentItemChanged);
        delete root;
        QCOMPARE(popup.parentItem(), nullptr);
        QCOMPARE(parentChanged.count(), 1);
    }
};

QTEST_MAIN(PopupAnchorTest)
#include "tst_popup_anchor.moc"
