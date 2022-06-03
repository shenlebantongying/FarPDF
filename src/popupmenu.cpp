#include "popupmenu.h"

popupMenu::popupMenu(QWidget * calling_widget, QWidget * parent)
    : QMenu(parent),
      calling_widget(calling_widget) {
}


void popupMenu::showEvent(QShowEvent * event) {
    auto cur = this->pos();

    move(cur.x() - this->width() / 2, cur.y());

    QWidget::showEvent(event);
}
