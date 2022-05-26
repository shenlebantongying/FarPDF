#ifndef FARMAINWINDOW_H
#define FARMAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QDockWidget>
#include "graphicsview.h"
#include "toctreemodel.h"
#include "document.h"

class farMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit farMainWindow(QWidget *parent = nullptr);

signals:

private:
    document * m_doc;
    GraphicsView * view;

    QDockWidget * toc_dock;
    tocTreeModel * toc;
    QTreeView * tocView;

    void jump_to_page(int n);
};

#endif // FARMAINWINDOW_H
