#ifndef HEADBAR_H
#define HEADBAR_H

#include <QEvent>
#include <QToolBar>
#include <QToolButton>
#include <QWindow>

class HeadBar : public QToolBar {
    Q_OBJECT
public:
    explicit HeadBar(QWidget * parent = nullptr);

    // maintain our own action list
    QList<QAction *> m_action;


    // give the caller a handle of the default action.

    QAction * addToolBtn(QAction * action);

    bool event(QEvent * event) override;

signals:
    void request_move_window();
    void request_window_resize(Qt::Edges edge);
};

#endif// HEADBAR_H
