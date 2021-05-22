#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QString::fromUtf8("Bilibili UWP视频重命名"));
    Widget w;
    w.setWindowTitle(QString::fromUtf8("Bilibili UWP视频重命名"));
    w.show();
    return a.exec();
}
