#include "application.h"
#include "utils/logger.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    try {
        CarlosOkupaApplication application;

        if (!application.initialize()) {
            Logger::log(Logger::Error, "Failed to initialize application");
            return 1;
        }

        application.show();

        int exitCode = app.exec();

        application.shutdown();
        return exitCode;
    }
    catch (const std::exception &e) {
        Logger::log(Logger::Error, QString("Unhandled exception: %1").arg(e.what()));
        QMessageBox::critical(nullptr, "Fatal Error",
            QString("Unhandled exception: %1").arg(e.what()));
        return 1;
    }
}
