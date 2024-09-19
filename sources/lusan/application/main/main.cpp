#include "lusan/application/main/Workspace.hpp"
#include "lusan/application/main/mainwindow.hpp"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Lusan_" + QLocale(locale).name();
        if (translator.load(":/ts/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    
    MainWindow w(nullptr);
    Workspace workspace;
    if (workspace.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        w.setWorkspaceRoot(workspace.getRootDirectory());
        w.show();
        return a.exec();
    }
    else
    {
        w.hide();
        return 0;
    }
}
