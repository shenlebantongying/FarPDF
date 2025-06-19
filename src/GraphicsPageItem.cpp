#include "GraphicsPageItem.h"

GraphicsPageItem::GraphicsPageItem(const QPixmap& pixmap, int page_number, QGraphicsItem* parent) :
    QGraphicsPixmapItem(pixmap, parent),
    page_num(page_number)
{
}
