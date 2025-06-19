#pragma once

#include <QGraphicsPixmapItem>

// QGraphicsPixmapItem but with page_num embeded

class GraphicsPageItem : public QGraphicsPixmapItem {
public:
    GraphicsPageItem(const QPixmap& pixmap, int page_number, QGraphicsItem* parent);
    int page_num;
};
