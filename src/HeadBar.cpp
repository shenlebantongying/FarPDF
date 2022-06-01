#include "HeadBar.h"
#include <QMimeData>
#include <QMouseEvent>

HeadBar::HeadBar(QWidget * parent)
    : QToolBar(parent) {
    setMovable(false);
    setFloatable(false);

    setAcceptDrops(true);

    setContextMenuPolicy(Qt::PreventContextMenu);
}

bool HeadBar::event(QEvent * event) {
    if (event->type() == QEvent::MouseButtonPress) {
        emit request_move_window();
    }

    return QToolBar::event(event);
}


QAction * HeadBar::addToolBtn(QAction * action) {
    auto tempBtn = new QToolButton();

    // Is it a bug or my understanding not enough?
    // After setting default action for QToolButton, the text will be lost.
    // Thus, we force set it here.
    tempBtn->setDefaultAction(action);
    tempBtn->defaultAction()->setText(action->text());

    this->addWidget(tempBtn);

    return tempBtn->defaultAction();
}