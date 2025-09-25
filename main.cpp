#include "mainwindow.h"
#include <QTimer>
#include <QApplication>
#include "Loading.h"
#include "Qylon.h"
#include <QDir>
#include <QStyleFactory>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include "Version.h"
int main(int argc, char *argv[])
{
    QString version = QString("1.0.1");
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

    MainWindow w;
    Loading l(&w, "Pixel Resolution Calculator", version);
    l.show();
    // QApplication::processEvents();

    w.setVersion(version + " " + QString(BUILD_VERSION).toUpper());
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
            QTimer::singleShot(250, [&l, &w, &ok] {
                w.activateWindow();
                w.show();
                w.raise();
                l.close();
                if(!ok) QMessageBox::warning(&w, w.windowTitle(), "Failed to load the default recipe.\nLoad the recipe manually.");
            });
            watcher->deleteLater();
        });

        const QString recipePath = QDir::toNativeSeparators("PRI.precipe");
        QFuture<bool> future = QtConcurrent::run([vtool, recipePath] {
            return vtool->loadRecipe(recipePath);
        });
        watcher->setFuture(future);
    });

    return a.exec();
}
