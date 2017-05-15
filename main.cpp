#include "r8asmwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;

    QCoreApplication::setOrganizationName("S.M.M.Software");
    QCoreApplication::setOrganizationDomain("m.m.shihov.com");
    QCoreApplication::setApplicationName("R8 Engine");

    QSettings settings;

    QString lang = settings.value("ui/language", "en_US").toString();
    if (lang == "ru_RU") {
        translator.load(":translate/ru_RU");
        a.installTranslator(&translator);
    }

    R8AsmWindow w;
    w.show();

    return a.exec();
}
