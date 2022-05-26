#include <QApplication>
#include "graphicsview.h"
#include "document.h"
#include "farmainwindow.h"
int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    auto far = new farMainWindow();
    far->show();

    return  QApplication::exec();
}
