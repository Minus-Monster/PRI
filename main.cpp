#include "mainwindow.h"
#include <QTimer>
#include <QApplication>
#include "Loading.h"
#include "Qylon.h"
#include <QDir>
#include <QStyleFactory>
#include <QtConcurrent/QtConcurrent>
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Resources);
    QApplication a(argc, argv);
    auto loadQSS = [](QString filePath) -> QString{
        QFile file(filePath);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            return QString();
        }
        QTextStream stream(&file);
        return stream.readAll();
    };

    Loading l;
    l.setName("Pixel Resolution Calculator");
    l.setVersion("Version 1.0");
    l.show();
    QApplication::processEvents();
    l.raise();
    l.activateWindow();

    MainWindow w;
    w.setStyleSheet(loadQSS(":/Resources/Style.qss"));

    Qylon::Qylon q;
    int number=0;
    QObject::connect(&q, &Qylon::Qylon::logMessage, &l, [&l, &number](QString message){
        number = number + 11;
        l.update(message, qMin(number, 89));
    });
    QTimer::singleShot(500, [&l, &q, &w]{
        l.update("Loaded the Qylon module.", 90);
        auto vtool = q.addVTools();

        l.update("Loading the vTools module...", 92);
        w.addVTools(vtool);
        l.update("Loading the recipe...", 95);


        auto watcher = new QFutureWatcher<bool>(&w);
        QObject::connect(watcher, &QFutureWatcher<bool>::finished, [&l, &w, watcher] {
            const bool ok = watcher->result();
            if (ok) {
                w.initialize();
                l.update("Loading succeeded.", 98);
            } else {
                l.update("Failed to load the recipe.", 98);
            }
            l.update("Finished loading all components.", 100);
            QTimer::singleShot(250, [&l, &w] {
                w.activateWindow();
                w.show();
                w.raise();
                l.close();
            });
            watcher->deleteLater();
        });

        const QString recipePath = QDir::toNativeSeparators("PRI.precipe");
        QFuture<bool> future = QtConcurrent::run([vtool, recipePath] {
            return vtool->loadRecipe(recipePath);
        });
        watcher->setFuture(future);


        // Recipe Loading
        /*
        if(vtool->loadRecipe("C:/Users/minwoo/Projects/PRI/PRI.precipe")){
        // if(vtool->loadRecipe("PRI.precipe")){
            w.initialize();
            l.update("Loading succeeded.", 95);
        }else{
            l.update("Failed to load the recipe.", 95);
        }







        l.update("Finished loading all components.", 100);
        QTimer::singleShot(250, [&l, &w]{
            w.activateWindow();
            w.show();
            w.raise();
            l.close();
        });
        */
    });

    return a.exec();
}
