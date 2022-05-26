#include "graphicsview.h"
#include <graphicspageitem.h>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>

GraphicsView::GraphicsView(Doc & doc_)
    :m_doc(doc_)
{
    scene = new QGraphicsScene();
    setScene(scene);
    scene->setSceneRect(0,0,m_doc.max_page_width,m_doc.page_acc_h.back());

    // The middle point of 1st page's boundary is (0,0)
    setAlignment(Qt::AlignTop);

    scene->setBackgroundBrush(Qt::lightGray);

    connect(this->verticalScrollBar(),&QScrollBar::valueChanged,
            [=,this]{this->make_sure_pages();});
}

void GraphicsView::addPage(int n){

    // Guard to avoid add duplicated page
    // Note: this can maybe optimized

    // there are a page already in rendered pages_nums

    if(std::find(rendered_pages_nums.begin(),rendered_pages_nums.end(),n) != rendered_pages_nums.end()){
        return; // just do nothing
    } else {
        rendered_pages_nums.push_back(n);

        auto t_pageItem = new GraphicsPageItem(m_doc.get_QPixmap_from_page_number(n), n, nullptr);
        t_pageItem->setOffset(0, m_doc.page_acc_h[n]);
        scene->addItem(t_pageItem);
    }
}

QList<QGraphicsItem*> GraphicsView::get_visible_page_items(){
    return scene->items(mapToScene(this->viewport()->geometry()).boundingRect());
}

void GraphicsView::mousePressEvent(QMouseEvent *event) {
    make_sure_pages();
    qDebug()<<get_visible_page_items();
}

std::vector<int> GraphicsView::demanded_page_numbers() {
     auto visable_rect = mapToScene(this->viewport()->geometry()).boundingRect();

     qreal top = visable_rect.top();    // smaller value
     qreal bot = visable_rect.bottom(); // bigger value
     int page_n_low;
     int page_n_high;

    // from head to end, find the first that bigger than top
    for (int i = 0; i < m_doc.page_acc_h.size(); ++i) {
        if (top<=m_doc.page_acc_h[i]){
            page_n_low = i-1;
            break;
        }
    }

    for (int i = m_doc.page_acc_h.size()-1 ; i >=0 ; --i) {
        if (bot>m_doc.page_acc_h[i]){
            page_n_high = i+1;
            break;
        }
    }

    // Guard to prevent requesting more pages than reality.
    if (page_n_high>m_doc.pageCount){
        page_n_high = m_doc.pageCount;
    }

    if (page_n_low<0){
        page_n_high = 0;
    }

    qDebug()<< page_n_low <<page_n_high;

    std::vector<int> v(page_n_high-page_n_low);
    std::iota (std::begin(v), std::end(v), page_n_low);;
    return v;
}

void GraphicsView::delete_pages(std::vector<int> pages) const{
    for (auto item:scene->items()) {
        if (item->type() == GraphicsPageItem::Type){
            auto y=dynamic_cast<GraphicsPageItem*>(item);
            if(std::find(pages.begin(), pages.end(), y->page_num) != pages.end()) {
                scene->removeItem(item);
                delete item;

            }
        }
    }
}

void GraphicsView::make_sure_pages(){
    auto demanded = demanded_page_numbers();

    for(auto x : demanded_page_numbers()){
        addPage(x);
    }

}