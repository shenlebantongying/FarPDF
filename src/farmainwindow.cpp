#include "farmainwindow.h"
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>

farMainWindow::farMainWindow(QWidget * parent) {
    // init code should not be related to specific doc,
    // set it to null to make sure the program will crash if some wrong code ever added;
    m_doc = nullptr;

    resize(1024, 800);

    view = new GraphicsView();
    setCentralWidget(view);

    //-- Table of contents -----------------------------------------------------------

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

    //-- Main Toolbar -----------------------------------------------------------------

    toolbar = new QToolBar();
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
    this->addToolBar(toolbar);

    // Open button
    const QIcon openIcon = QIcon::fromTheme("document-open");
    auto * openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));

    connect(openAct, &QAction::triggered, [=, this] {
        load_document();
    });

    toolbar->addAction(openAct);

    // Page Indicator
    auto page_indicator = new QLabel("0", this);
    toolbar->addSeparator();
    toolbar->addWidget(page_indicator);
    page_indicator->setMinimumWidth(20);
    page_indicator->setAlignment(Qt::AlignCenter);

    connect(view, &GraphicsView::page_updated,
            [=, this] {
                page_indicator->setNum(view->get_middle_page_num());
            });

    // Zoom switcher
    auto zoom_switcher = new QComboBox(this);
    zoom_switcher->addItem(" 50%", 0.5);
    zoom_switcher->addItem("100%", 1.0);
    zoom_switcher->addItem("150%", 1.5);
    zoom_switcher->addItem("200%", 2.0);
    zoom_switcher->setCurrentText("100%");

    toolbar->addWidget(zoom_switcher);

    connect(zoom_switcher, &QComboBox::currentIndexChanged,
            [=, this] {
                view->zoom_to(zoom_switcher->currentData().toFloat());
            });
}

void farMainWindow::jump_to_page(int n) {
    view->centerOn(0, (m_doc->page_acc_h.at(n + 1) + m_doc->page_acc_h.at(n)) / 2.0);
}

void farMainWindow::load_document() {
    delete m_doc;
    QString file_name = QFileDialog::getOpenFileName(this,
                                                     "Open File",
                                                     QDir::homePath(),
                                                     "Documents (*.pdf)");
    if (file_name.isEmpty() or file_name.isNull()) {
        return;
    }

    m_doc = new document(file_name.toStdString());

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