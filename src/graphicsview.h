#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QQueue>

#include "document.h"
#include "graphicspageitem.h"

class GraphicsView : public QGraphicsView {
    Q_OBJECT

public:
    GraphicsView();
    QGraphicsScene * scene;
    void update_doc(document * doc_);

    int get_middle_page_num();
    void zoom_to(float factor);

signals:
    void page_updated();

private:
    document * m_doc;

    void addPage(int n);

    // pages that are visible to users;
    std::vector<int> demanded_page_numbers();

    // Keep track of rendered pages, if they are not visible, then remove some.

    QQueue<GraphicsPageItem *> live_pages;

    QList<QGraphicsItem *> rendered_pages;
    std::vector<int> rendered_pages_nums;

    QList<QGraphicsItem *> get_visible_page_items();

    void mousePressEvent(QMouseEvent * event) override;

    void make_sure_pages();
};

#endif// GRAPHICSVIEW_H
