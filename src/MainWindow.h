#pragma once

#include "Document.h"
#include "GraphicsView.h"
#include "TocTreeModel.h"
#include <QComboBox>
#include <QDockWidget>
#include <QMainWindow>
#include <QToolBar>
#include <QTreeView>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    void load_document_from_path(const QString& filename);

signals:

private:
    Document* m_doc;
    GraphicsView* view;

    QList<float> zoom_leveler;
    QComboBox* zoom_switcher;

    QDockWidget* toc_dock;
    TocTreeModel* toc = nullptr;
    QTreeView* tocView;

    QToolBar* toolbar;

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
