#include "graphicsview.h"
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>

GraphicsView::GraphicsView() {
    m_doc = nullptr;
    scene = new QGraphicsScene();

    // This enable mouse selection
    setMouseTracking(true);

    zoom_factor = 1;

    setScene(scene);


    // The middle point of 1st page's boundary is (0,0)
    setAlignment(Qt::AlignTop);

    scene->setBackgroundBrush(Qt::lightGray);
    connect(this->verticalScrollBar(),
            &QScrollBar::valueChanged,
            [=, this] {
                this->make_sure_pages();
                emit page_updated();
            });

    reset();
}

void GraphicsView::update_doc(document * doc_) {
    m_doc = doc_;

    reset();

    // TODO: temporal hack, just get first page's width and consider it the whole doc.
    addPage(0);
    scene->setSceneRect(0, 0, scene->itemsBoundingRect().width(), zoom_factor * m_doc->page_acc_h.back());

    make_sure_pages();
}

void GraphicsView::addPage(int n) {
    if (live_pages.size() > 10) {
        auto to_be_del = live_pages.dequeue();
        scene->removeItem(to_be_del);
        delete to_be_del;
    }

    // Guard to avoid duplicated page rendering
    for (auto i: live_pages) {
        if (i->page_num == n) {
            return;
        }
    }

    auto t_pageItem = new GraphicsPageItem(m_doc->get_QPixmap_from_page_number(n, zoom_factor), n, nullptr);
    t_pageItem->setOffset(0, m_doc->page_acc_h[n] * zoom_factor);

    scene->addItem(t_pageItem);
    live_pages.enqueue(t_pageItem);
}

QList<QGraphicsItem *> GraphicsView::get_visible_page_items() {
    return scene->items(mapToScene(this->viewport()->geometry()).boundingRect());
}

std::vector<int> GraphicsView::demanded_page_numbers() {
    auto visable_rect = mapToScene(this->viewport()->geometry()).boundingRect();

    qreal top = visable_rect.top();   // smaller value
    qreal bot = visable_rect.bottom();// bigger value
    int page_n_low;
    unsigned long page_n_high;

    // from head to end, find the first that bigger than top
    for (int i = 0; i < m_doc->page_acc_h.size(); ++i) {
        if (top <= zoom_factor * m_doc->page_acc_h[i]) {
            page_n_low = i - 1;
            break;
        }
    }

    for (unsigned long i = m_doc->page_acc_h.size() - 1; i >= 0; --i) {
        if (bot > zoom_factor * m_doc->page_acc_h[i]) {
            page_n_high = i + 1;
            break;
        }
    }

    // Guard to prevent requesting more pages than reality.
    if (page_n_high > m_doc->pageCount) {
        page_n_high = m_doc->pageCount;
    }

    if (page_n_low < 0) {
        page_n_high = 0;
    }

    std::vector<int> v(page_n_high - page_n_low);
    std::iota(std::begin(v), std::end(v), page_n_low);
    return v;
}

void GraphicsView::make_sure_pages() {
    for (auto x: demanded_page_numbers()) {
        addPage(x);
    }
}

int GraphicsView::get_middle_page_num() {
    auto height = mapToScene(this->viewport()->geometry().center()).y();

    for (int i = 0; i < m_doc->pageCount; ++i) {
        if (height < m_doc->page_acc_h[i]) {
            return i;
        }
    }

    return 0;
}

void GraphicsView::zoom_to(float factor) {
    zoom_factor = factor;

    reset();

    addPage(0);

    scene->setSceneRect(0, 0, zoom_factor * scene->itemsBoundingRect().width(), zoom_factor * m_doc->page_acc_h.back());
    make_sure_pages();
}

void GraphicsView::jump_to_page(int n) {
    centerOn(0, zoom_factor * (m_doc->page_acc_h.at(n + 1) + m_doc->page_acc_h.at(n)) / 2.0);
}

// -------------------------------------------------------
//    Mouse Movement handling
// -------------------------------------------------------

void GraphicsView::mousePressEvent(QMouseEvent * event) {
    for (auto x: select_group->childItems()) {
        select_group->removeFromGroup(x);
        delete x;
    }

    dragBeg_P = event->pos();

    select_rect->setVisible(true);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent * event) {
    select_rect->setVisible(false);
    select_rect->setPos(0, 0);

    // TODO: this is buggy and only works on single page

    QRectF c_rect = mapToScene(QRect(dragBeg_P, dragEnd_P)).boundingRect();


    if (itemAt(dragBeg_P) == itemAt(dragEnd_P)) {
        auto it = dynamic_cast<GraphicsPageItem *>(itemAt(dragBeg_P));
        float y_off = m_doc->page_acc_h[it->page_num];
        auto hls = new QList<QRectF>();


        m_doc->highlight_selection(it->page_num,
                                   QPointF(c_rect.topLeft().x() / zoom_factor, c_rect.topLeft().y() / zoom_factor - y_off),
                                   QPointF(c_rect.bottomRight().x() / zoom_factor, c_rect.bottomRight().y() / zoom_factor - y_off),
                                   *hls);

        for (auto r: *hls) {
            auto temp_rect = new QGraphicsRectItem(r.x() * zoom_factor, zoom_factor * (r.y() + y_off),
                                                   r.width() * zoom_factor, r.height() * zoom_factor);
            select_group->addToGroup(temp_rect);
        }
    }
}

void GraphicsView::mouseMoveEvent(QMouseEvent * event) {
    if (select_rect->isVisible()) {
        dragEnd_P = event->pos();

        double dx = dragEnd_P.x() - dragBeg_P.x();
        double dy = dragEnd_P.y() - dragBeg_P.y();

        double width = abs(dx);
        double height = abs(dy);

        if (dx > 0 && dy > 0) {
            select_rect->setPolygon(mapToScene(QRect(dragBeg_P.x(), dragBeg_P.y(), width, height)));
        } else if (dx < 0 && dy > 0) {
            select_rect->setPolygon(mapToScene(QRect(dragEnd_P.x(), dragBeg_P.y(), width, height)));
        } else if (dx > 0 && dy < 0) {
            select_rect->setPolygon(mapToScene(QRect(dragBeg_P.x(), dragEnd_P.y(), width, height)));
        } else {
            select_rect->setPolygon(mapToScene(QRect(dragEnd_P.x(), dragEnd_P.y(), width, height)));
        }
    }
}

void GraphicsView::reset() {
    scene->clear();
    live_pages.clear();


    // selection highlights
    select_group = new QGraphicsItemGroup();
    select_group->setZValue(90);
    scene->addItem(select_group);


    // select_rect rectangle
    select_rect = new QGraphicsPolygonItem();
    select_rect->setVisible(false);
    select_rect->setZValue(100);
    scene->addItem(select_rect);
}