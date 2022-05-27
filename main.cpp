#include "farmainwindow.h"
#include <QApplication>
int main(int argc, char * argv[]) {
    QApplication a(argc, argv);

    auto far = new farMainWindow();
    far->show();

    return QApplication::exec();
}
