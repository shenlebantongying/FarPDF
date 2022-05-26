#include <QFileDialog>
#include "farmainwindow.h"


farMainWindow::farMainWindow(QWidget *parent)
{
    m_doc = new document(QFileDialog::getOpenFileName(this,
                                                      "Open File",
                                                      QDir::homePath(),
                                                      "Documents (*.pdf)")
                                 .toStdString());

    view = new GraphicsView(*m_doc);

    toc_dock = new QDockWidget("Table of Contents", this);
    tocView = new QTreeView();
    toc = nullptr;

    setCentralWidget(view);


    // Note: we will take the dock to a standalone one later
    addDockWidget(Qt::LeftDockWidgetArea, toc_dock);
    toc_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    toc_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    toc_dock->setContextMenuPolicy(Qt::PreventContextMenu);

    if (!toc){
        toc = new tocTreeModel(m_doc->get_outline());
    } else {
        toc->update_outline(m_doc->get_outline());
    }

    tocView->setHeaderHidden(true);
    tocView->setModel(toc);

    tocView->expandAll();
    toc_dock->setWidget(tocView);

    connect(tocView->selectionModel(),
            &QItemSelectionModel::currentChanged,
            // TODO: side effect -> open a doc will always end up in the 1st bookmark
            [=, this](const QModelIndex &current, const QModelIndex &previous) {
                jump_to_page(toc->page_num_from_index(current));
            });
}

void farMainWindow::jump_to_page(int n) {
    view->centerOn(0, (m_doc->page_acc_h.at(n+1) + m_doc->page_acc_h.at(n)) / 2.0);
}

