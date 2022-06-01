#include "farmainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QHoverEvent>
#include <QLabel>
#include <QShortcut>
#include <QSizePolicy>
#include <array>

constexpr qreal border = 15;
constexpr int w = 2;

farMainWindow::farMainWindow(QWidget * parent)
    : QMainWindow(parent) {

    // init code should not be related to specific doc,
    // set it to null to make sure the program will crash if some wrong code ever added;
    m_doc = nullptr;

    // CSD_BEGIN
    setWindowFlag(Qt::FramelessWindowHint);
    setContentsMargins(w, w, w, w);
    // Enable hove move event -> to change cursor when near window boarder.
    setAttribute(Qt::WA_Hover);
    // CSD_END

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

    headbar = new HeadBar();

    // if the headbar request win_move (which will be emitted by mouse click event)
    connect(headbar, &HeadBar::request_move_window,
            [=, this] {
                this->windowHandle()->startSystemMove();
            });

    // close, fullscreen, and close

    auto close_ac = headbar->addToolBtn(new QAction(QIcon::fromTheme("window-close"), "Close"));
    connect(close_ac, &QAction::triggered,
            [=, this]() {
                QApplication::quit();
            });

    auto fullscreen_ac = headbar->addToolBtn(new QAction(QIcon::fromTheme("window-maximize"), ("fullscreen")));
    connect(fullscreen_ac, &QAction::triggered,
            [=, this] {
                if (windowState() == Qt::WindowMaximized) {
                    setWindowState(Qt::WindowNoState);
                } else {
                    setWindowState(Qt::WindowMaximized);
                }
            });

    auto minimize_ac = headbar->addToolBtn(new QAction(QIcon::fromTheme("window-minimize"), "Minimize"));
    connect(minimize_ac, &QAction::triggered,
            [=, this]() {
                setWindowState(Qt::WindowMinimized);
            });


    this->addToolBar(headbar);

    // Open button
    const QIcon openIcon = QIcon::fromTheme("document-open");
    auto * openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));

    connect(openAct, &QAction::triggered, [=, this] {
        load_document();
    });

    headbar->addAction(openAct);

    auto blank_separator = new QWidget();
    blank_separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    headbar->addWidget(blank_separator);

    // Page Indicator
    auto page_indicator = new QLabel("0", this);
    headbar->addWidget(page_indicator);
    page_indicator->setMinimumWidth(20);
    page_indicator->setAlignment(Qt::AlignCenter);

    // MetaData Dialog
    auto * infoAct = new QAction(QIcon::fromTheme("documentinfo"), tr("&Open..."), this);

    connect(infoAct, &QAction::triggered, [=, this] {
        show_metadata_dialog();
    });

    headbar->addAction(infoAct);

    connect(view, &GraphicsView::page_updated,
            [=, this] {
                page_indicator->setNum(view->get_middle_page_num());
            });

    // Zoom switcher
    zoom_switcher = new QComboBox();
    zoom_leveler = {3.0, 2.75, 2.50, 2.25, 2.0, 1.75, 1.50, 1.40, 1.30, 1.20, 1.10, 1.0, 0.9, 0.75, 0.5};
    for (auto x: zoom_leveler) {
        zoom_switcher->addItem(QString::number(x * 100) + "%", x);
    }

    zoom_switcher->setCurrentText("100%");

    headbar->addWidget(zoom_switcher);

    connect(zoom_switcher, &QComboBox::currentIndexChanged,
            [=, this] {
                if (m_doc != nullptr) {
                    view->zoom_to(zoom_switcher->currentData().toFloat());
                }
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

    connect(tocView->selectionModel(),
            &QItemSelectionModel::currentChanged,
            // TODO: side effect -> open a doc will always end up in the 1st bookmark
            [=, this](const QModelIndex & current, const QModelIndex & /*previous*/) {
                jump_to_page(tocTreeModel::page_num_from_index(current));
            });
}

void farMainWindow::show_metadata_dialog() {
    if (m_doc) {
        auto metaDialog = new QLabel();
        metaDialog->setTextInteractionFlags(Qt::TextSelectableByMouse);
        metaDialog->setMargin(10);
        metaDialog->setText(QString::fromStdString(m_doc->get_metadata_string()));
        metaDialog->setAttribute(Qt::WA_DeleteOnClose);
        metaDialog->show();
    }
}


bool farMainWindow::event(QEvent * event) {

    // When hove to edge, change cursor
    if (event->type() == QEvent::HoverMove) {
        auto p = dynamic_cast<QHoverEvent *>(event)->position();

        if (p.y() > height() - border) {
            if (p.x() < border) {
                this->setCursor(Qt::SizeBDiagCursor);
            } else if (p.x() > width() - border) {
                this->setCursor(Qt::SizeFDiagCursor);
            } else {
                this->setCursor(Qt::SizeVerCursor);
            }
        } else if ((p.y() > headbar->height()) and
                   (((p.x() > width() - border) or (p.x() < border)))) {
            this->setCursor(Qt::SizeHorCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
        // if user press
    } else if (event->type() == QEvent::MouseButtonPress) {
        auto p = dynamic_cast<QMouseEvent *>(event)->position();
        Qt::Edges edges;
        if (p.x() > width() - border) {
            edges |= Qt::RightEdge;
        }
        if (p.x() < border) {
            edges |= Qt::LeftEdge;
        }
        if (p.y() > height() - border) {
            edges |= Qt::BottomEdge;
        }
        if (p.y() < border) {
            edges |= Qt::TopEdge;
        }

        if (edges != 0) {
            // Note: on Mac, this will return false which means isn't supported.
            this->windowHandle()->startSystemResize(edges);
        }
    }
    // TODO: what does this mean?
    return QMainWindow::event(event);
}

void farMainWindow::paintEvent(QPaintEvent *) {
    auto pen = new QPen();
    pen->setWidth(w * 2);
    pen->setColor(Qt::lightGray);
    QPainter p(this);
    p.setPen(*pen);
    p.drawRect(this->rect());
}
