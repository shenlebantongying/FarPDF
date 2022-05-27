#include <QFileDialog>
#include <QLabel>

#include "farmainwindow.h"

farMainWindow::farMainWindow(QWidget * parent) {
    // init code should not related to specific doc,
    // set it to null to make sure the program will crash;
    m_doc = nullptr;

    view = new GraphicsView();
    setCentralWidget(view);

    setMinimumSize(800, 600);

    toc_dock = new QDockWidget("Table of Contents", this);
    tocView = new QTreeView();
    toc = nullptr;

    // Note: we will take the dock to a standalone one later
    addDockWidget(Qt::LeftDockWidgetArea, toc_dock);
    toc_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    toc_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    toc_dock->setContextMenuPolicy(Qt::PreventContextMenu);

    tocView->setHeaderHidden(true);
    tocView->setModel(toc);
    tocView->expandAll();
    toc_dock->setWidget(tocView);
    toc_dock->hide();

    // Main Toolbar

    toolbar = new QToolBar();
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    this->addToolBar(toolbar);


    const QIcon openIcon = QIcon::fromTheme("document-open");
    auto * openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));

    connect(openAct, &QAction::triggered, [=, this] {
        load_document();
    });

    toolbar->addAction(openAct);


    auto page_indicator = new QLabel("0");
    toolbar->addSeparator();
    toolbar->addWidget(page_indicator);

    connect(view, &GraphicsView::page_updated,
            [=, this] {
                page_indicator->setText(QString::number(view->get_middle_page_num()));
            });
}

void farMainWindow::jump_to_page(int n) {
    view->centerOn(0, (m_doc->page_acc_h.at(n + 1) + m_doc->page_acc_h.at(n)) / 2.0);
}

void farMainWindow::load_document() {
    delete m_doc;
    m_doc = new document(QFileDialog::getOpenFileName(this,
                                                      "Open File",
                                                      QDir::homePath(),
                                                      "Documents (*.pdf)")
                                 .toStdString());

    view->update_doc(m_doc);

    delete toc;
    toc = new tocTreeModel(m_doc->get_outline());

    tocView->setModel(toc);
    toc_dock->show();

    connect(tocView->selectionModel(),
            &QItemSelectionModel::currentChanged,
            // TODO: side effect -> open a doc will always end up in the 1st bookmark
            [=, this](const QModelIndex & current, const QModelIndex & previous) {
                jump_to_page(toc->page_num_from_index(current));
            });
}
