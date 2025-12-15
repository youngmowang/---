#include "ui/MainWindow.h"
#include "db/DatabaseManager.h"
#include "ui/ThemeManager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#endif



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString dbPath = dir.filePath("autoword.db");
    
    if (!DatabaseManager::instance().connect(dbPath)) {
        qCritical() << "Could not connect to database at" << dbPath;
        return -1;
    }

    ThemeManager::instance();

    MainWindow window;
    window.show();

    return app.exec();
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = 1;
    char* argv[] = { (char*)"AutoWord", nullptr };
    return main(argc, argv);
}
#endif
