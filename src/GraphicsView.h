#pragma once

#include "Document.h"
#include "GraphicsPageItem.h"
#include <QGraphicsView>
#include <QQueue>

class GraphicsView : public QGraphicsView {
    Q_OBJECT

public:
    GraphicsView();
    QGraphicsScene* scene;
    void update_doc(Document* doc_);

    bool fit_to_width_q;
    qreal raw_page_width;

    int get_middle_page_num();
    void jump_to_page(int user_facing_page_number);
    void resizeEvent(QResizeEvent* event) override;

    void add_search_rect_at_page(QRectF rect, int page_num);
    void clear_search_rect();

signals:
    void page_updated();

public slots:
    void zoom_to(float factor);

private:
    Document* m_doc;
    float zoom_factor;

    QGraphicsPolygonItem* select_rect;
    QPoint dragBeg_P;
    QPoint dragEnd_P;

    QGraphicsItemGroup* select_group;
    QGraphicsItemGroup* search_group;

    void addPage(int n);

    // pages that are visible to users;
    std::vector<int> demanded_page_numbers();

    // Keep track of rendered pages, if they are not visible, then remove some.

    QQueue<GraphicsPageItem*> live_pages;

    QList<QGraphicsItem*> rendered_pages;
    std::vector<int> rendered_pages_nums;

    QList<QGraphicsItem*> get_visible_page_items();

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void make_sure_pages();
    void reset();

    // Helpers

    /**
     * @brief Zoomify for continuous scrolling.
     * @param rect
     * @param page_num
     * @return
     */
    QRectF zoomify_rect_to_page(const QRectF& rect, int page_num);
};
