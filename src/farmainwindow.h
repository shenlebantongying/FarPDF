#ifndef FARMAINWINDOW_H
#define FARMAINWINDOW_H

#include <QComboBox>
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

    void load_document_from_path(const QString & filename);

signals:

private:
    document * m_doc;
    GraphicsView * view;

    QList<float> zoom_leveler;
    QComboBox * zoom_switcher;

    QDockWidget * toc_dock;
    tocTreeModel * toc;
    QTreeView * tocView;

    QToolBar * toolbar;

    void jump_to_page(int n);

    void load_document();
    void show_metadata_dialog();


    /**
     * @brief Change view's zooming factor
     * @param factor
     */
    void zoom_switch(float factor);

    void zoom_up();
    void zoom_down();
};

#endif// FARMAINWINDOW_H
