#include <QtWidgets/QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLabel *label = new QLabel("yeeee");
    QPixmap *pixmap = new QPixmap("/home/user/pbm.pbm");
    
    label->setPixmap(*pixmap);
    label->show();

    return app.exec();
}