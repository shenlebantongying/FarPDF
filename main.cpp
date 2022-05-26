#include <QApplication>
#include "graphicsview.h"
#include "Doc.h"
int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    const char* doc_name = "test.pdf";
    auto doc =  Doc(doc_name);
    auto view = new GraphicsView(doc);
    view->show();

    return  QApplication::exec();
}
