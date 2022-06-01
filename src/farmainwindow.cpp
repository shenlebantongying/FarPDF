#include "farmainwindow.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QLabel>
#include <QResizeEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <array>

farMainWindow::farMainWindow(QWidget * parent)
    : QMainWindow(parent) {

    // init code should not be related to specific doc,
    // set it to null to make sure the program will crash if some wrong code ever added;
    m_doc = nullptr;

    resize(1024, 800);

    view = new GraphicsView();

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


    // MetaData Dialog
    auto * infoAct = new QAction(QIcon::fromTheme("documentinfo"), tr("&Open..."), this);

    connect(infoAct, &QAction::triggered, [=, this] {
        show_metadata_dialog();
    });


    // Page Indicator
    auto page_indicator = new QLabel("0", this);
    page_indicator->setMinimumWidth(20);
    page_indicator->setAlignment(Qt::AlignCenter);

    connect(view, &GraphicsView::page_updated,
            [=, this] {
                page_indicator->setText(QString::number(view->get_middle_page_num()) + "/" + QString::number(m_doc->pageCount) + " ");
            });

    // Zoom switcher
    zoom_switcher = new QComboBox();
    zoom_leveler = {3.0, 2.75, 2.50, 2.25, 2.0, 1.75, 1.50, 1.40, 1.30, 1.20, 1.10, 1.0, 0.9, 0.75, 0.5};
    for (auto x: zoom_leveler) {
        zoom_switcher->addItem(QString::number(x * 100) + "%", x);
    }

    zoom_switcher->addItem(QString("Fit to width"), "fit_to_width");

    zoom_switcher->setCurrentText("100%");


    connect(zoom_switcher, &QComboBox::currentIndexChanged,
            [=, this] {
                const std::string t_float("float");
                if (zoom_switcher->currentData().typeName() == t_float) {
                    if (m_doc != nullptr) {
                        view->fit_to_width_q = false;
                        view->zoom_to(zoom_switcher->currentData().toFloat());
                    }
                } else {
                    view->fit_to_width_q = true;
                    // manually trigger reisze event
                    emit view->resizeEvent(new QResizeEvent(view->size(), view->size()));
                };
            });

    auto zoom_up = new QShortcut(QKeySequence(QKeySequence::ZoomOut), this);
    auto zoom_down = new QShortcut(QKeySequence(QKeySequence::ZoomIn), this);

    connect(zoom_up, &QShortcut::activated,
            [=, this] {
                this->zoom_up();
            });
    connect(zoom_down, &QShortcut::activated,
            [=, this] {
                this->zoom_down();
            });

    // Centralized place to make toolbar
    toolbar->addAction(openAct);
    toolbar->addAction(infoAct);

    auto blank_splitter = new QWidget(this);
    blank_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    toolbar->addWidget(blank_splitter);

    toolbar->addWidget(page_indicator);
    toolbar->addWidget(zoom_switcher);
}

void farMainWindow::zoom_switch(float factor) {
    view->zoom_to(factor);
};

void farMainWindow::zoom_up() {
    auto new_index = -1 + zoom_switcher->currentIndex();
    if (new_index >= 0) {
        zoom_switcher->setCurrentIndex(new_index);
    }
}

void farMainWindow::zoom_down() {
    auto new_index = 1 + zoom_switcher->currentIndex();
    if (new_index < zoom_switcher->count()) {
        zoom_switcher->setCurrentIndex(new_index);
    }
}

void farMainWindow::jump_to_page(int n) {
    view->jump_to_page(n);
}

void farMainWindow::load_document() {
    delete m_doc;
    QString file_name = QFileDialog::getOpenFileName(this,
                                                     "Open File",
                                                     QDir::homePath(),
                                                     "Documents (*.pdf)");
    load_document_from_path(file_name);
}

void farMainWindow::load_document_from_path(const QString & file_name) {

    setWindowTitle(file_name);

    if (file_name.isEmpty() or file_name.isNull()) {
        return;
    }

    m_doc = new document(file_name.toStdString());

    view->update_doc(m_doc);

    if (centralWidget() == nullptr) {
        setCentralWidget(view);
    }

    delete toc;
    toc = new tocTreeModel(m_doc->get_outline());

    tocView->setModel(toc);
    toc_dock->show();
    toc_dock->setMinimumWidth(100);
    toc_dock->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    connect(tocView->selectionModel(),
            &QItemSelectionModel::currentChanged,
            // TODO: side effect -> open a doc will always end up in the 1st bookmark
            [=, this](const QModelIndex & current, const QModelIndex & /*previous*/) {
                jump_to_page(tocTreeModel::page_num_from_index(current));
            });
}

void farMainWindow::show_metadata_dialog() {
    if (m_doc) {
        // Note: Qt::Tool + parent set properly = always on top of the parent.
        auto metaDialog = new QLabel(this);
        metaDialog->setWindowFlags(Qt::Tool);

        metaDialog->setTextInteractionFlags(Qt::TextSelectableByMouse);
        metaDialog->setMargin(10);
        metaDialog->setText(QString::fromStdString(m_doc->get_metadata_string()));
        metaDialog->setAttribute(Qt::WA_DeleteOnClose);
        metaDialog->show();
    }
}