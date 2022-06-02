#include "graphicsview.h"
#include <QApplication>
#include <QClipboard>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QShortcut>

GraphicsView::GraphicsView() {
    m_doc = nullptr;
    scene = new QGraphicsScene();

    // This enable mouse selection
    setMouseTracking(true);

    zoom_factor = 1;

    setScene(scene);

    fit_to_width_q = false;

    // The middle point of 1st page's boundary is (0,0)
    setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    scene->setBackgroundBrush(Qt::lightGray);

    connect(this->verticalScrollBar(),
            &QScrollBar::valueChanged,
            [=, this] {
                this->make_sure_pages();
                emit page_updated();
            });

    QClipboard * clip = QApplication::clipboard();

    auto copy = new QShortcut(QKeySequence(QKeySequence::Copy), this);
    connect(copy, &QShortcut::activated,
            [=, this] {
                auto c_rect = mapToScene(QRect(dragBeg_P, dragEnd_P)).boundingRect();


                /// TODO: convert "obtain page_num by height" to a func

                auto normalized_h = (float)c_rect.y() * zoom_factor;
                auto index_it = std::find_if(m_doc->page_acc_h.begin(), m_doc->page_acc_h.end(),
                                             [normalized_h](float n) {
                                                 return (n > normalized_h);
                                             });

                auto page_num = -1 + index_it - m_doc->page_acc_h.begin();


                float y_off = m_doc->page_acc_h[page_num];

                auto str = m_doc->get_selection_text((int)page_num,
                                                     QPointF(c_rect.topLeft().x() / zoom_factor, c_rect.topLeft().y() / zoom_factor - y_off),
                                                     QPointF(c_rect.bottomRight().x() / zoom_factor, c_rect.bottomRight().y() / zoom_factor - y_off));

                if (!(str.isEmpty() || str.isNull())) {
                    clip->setText(str);
                }
            });


    reset();
}

void GraphicsView::update_doc(document * doc_) {
    m_doc = doc_;

    reset();

    // TODO: temporal hack, just get first page's width and consider it the whole doc.
    addPage(0);
    raw_page_width = scene->itemsBoundingRect().width();
    this->setSceneRect(0, 0, zoom_factor * raw_page_width, zoom_factor * m_doc->page_acc_h.back());

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
    int page_n_low = 0;
    int page_n_high = 0;

    // from head to end, find the first that bigger than top
    for (auto i = 0u; i < m_doc->page_acc_h.size(); ++i) {
        if (top <= zoom_factor * m_doc->page_acc_h[i]) {
            page_n_low = (int)i - 1;
            break;
        }
    }

    for (auto i = m_doc->page_acc_h.size() - 1; i > 0; --i) {
        if (bot > zoom_factor * m_doc->page_acc_h[i]) {
            page_n_high = (int)i + 1;
            break;
        }
    }

    // Guards to prevent requesting more pages than reality.

    if (page_n_low == page_n_high) {
        page_n_high += 1;
        page_n_low -= 1;
    }

    if (page_n_high > m_doc->pageCount) {
        page_n_high = m_doc->pageCount;
    }

    if (page_n_low < 0) {
        page_n_low = 0;
    }

    //

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

    // auto curpage = get_middle_page_num();

    reset();

    addPage(0);

    this->setSceneRect(0, 0, zoom_factor * scene->itemsBoundingRect().width(), zoom_factor * m_doc->page_acc_h.back());
    make_sure_pages();
}

void GraphicsView::jump_to_page(int user_facing_page_number /* aka 1-based page numbering*/) {
    centerOn(0, zoom_factor * (m_doc->page_acc_h.at(user_facing_page_number - 1) +
                               (m_doc->get_page_height(user_facing_page_number - 1)) / 2.0));
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

void GraphicsView::mouseReleaseEvent(QMouseEvent *) {
    select_rect->setVisible(false);
    select_rect->setPos(0, 0);

    // TODO: this is buggy and only works on single page


    if (itemAt(dragBeg_P) == itemAt(dragEnd_P) && itemAt(dragBeg_P) != nullptr && itemAt(dragEnd_P) != nullptr) {

        QRectF c_rect = mapToScene(QRect(dragBeg_P, dragEnd_P)).boundingRect();

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

void GraphicsView::resizeEvent(QResizeEvent * event) {
    if (fit_to_width_q) {
        zoom_factor = (float)(event->size().width() / raw_page_width);
        zoom_to(zoom_factor);
    }

    // Note: werid hack, to make sure pages are always horizontally centered after zooming or window width change
    setSceneRect(0, 0, raw_page_width * zoom_factor, zoom_factor * m_doc->page_acc_h.back());
}