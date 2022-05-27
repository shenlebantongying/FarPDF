#include "graphicsview.h"
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>

GraphicsView::GraphicsView() {
    m_doc = nullptr;
    scene = new QGraphicsScene();

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
}

void GraphicsView::update_doc(document * doc_) {
    m_doc = doc_;
    scene->clear();
    live_pages.clear();

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

void GraphicsView::mousePressEvent(QMouseEvent * event) {
    // make_sure_pages();
    // qDebug()<<get_visible_page_items();
}

std::vector<int> GraphicsView::demanded_page_numbers() {
    auto visable_rect = mapToScene(this->viewport()->geometry()).boundingRect();

    qreal top = visable_rect.top();   // smaller value
    qreal bot = visable_rect.bottom();// bigger value
    int page_n_low;
    int page_n_high;

    // from head to end, find the first that bigger than top
    for (int i = 0; i < m_doc->page_acc_h.size(); ++i) {
        if (top <= zoom_factor * m_doc->page_acc_h[i]) {
            page_n_low = i - 1;
            break;
        }
    }

    for (int i = m_doc->page_acc_h.size() - 1; i >= 0; --i) {
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
    auto item = scene->itemAt(mapToScene(this->viewport()->geometry().center()),
                              QTransform());
    if (item) {
        // Note: the cast should be avoided if the scene is subclassed
        return dynamic_cast<GraphicsPageItem *>(item)->page_num;
    } else {
        return 0;
    }
}

void GraphicsView::zoom_to(float factor) {
    zoom_factor = factor;

    scene->clear();
    live_pages.clear();
    addPage(0);

    scene->setSceneRect(0, 0, zoom_factor * scene->itemsBoundingRect().width(), zoom_factor * m_doc->page_acc_h.back());
    make_sure_pages();
}

void GraphicsView::jump_to_page(int n) {
    centerOn(0, zoom_factor * (m_doc->page_acc_h.at(n + 1) + m_doc->page_acc_h.at(n)) / 2.0);
}