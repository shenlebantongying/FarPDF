#ifndef FARMAINWINDOW_H
#define FARMAINWINDOW_H

#include <QDockWidget>
#include <QMainWindow>
#include <QToolBar>
#include <QTreeView>

#include "document.h"
#include "graphicsview.h"
#include "toctreemodel.h"

class farMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit farMainWindow(QWidget * parent = nullptr);

signals:

private:
    document * m_doc;
    GraphicsView * view;

    QDockWidget * toc_dock;
    tocTreeModel * toc;
    QTreeView * tocView;

    QToolBar * toolbar;

    void jump_to_page(int n);

    void load_document();
};

#endif// FARMAINWINDOW_H
