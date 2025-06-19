#include "MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    auto far = new MainWindow();

    if (argc == 2) {
        far->load_document_from_path(argv[1]);
    }

    far->show();

    return QApplication::exec();
}
