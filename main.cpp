#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icon/icon.ico")));

    MainWindow w;

    const QStringList args = QApplication::arguments();

    for(const QString &arg : args) {
        if(arg.startsWith(QStringLiteral("--lang="))) {
            w.set_language(arg.section(QLatin1Char('='), 1));
            break;
        }
    }

    w.show();
    return QApplication::exec();
}
