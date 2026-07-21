#include "application.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    CarlosOkupaApplication application;
    application.initialize();
    application.show();
    
    return app.exec();
}
