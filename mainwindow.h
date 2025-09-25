#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Qylon/vTools/vTools.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addVTools(Qylon::vTools *v);
    void initialize();

    QStringList getCameraList();
    QString getCurrentCamera();
    bool setCurrentCamera(QString cameraSerial);

    int addTableWidget(QString serialNum);
    QStringList dataCollection();

    void builderRecipe();
    void checkRecipe();
    void setVersion(QString vers);
    // QGraphicsEllipseItem* leftTop;
    // QGraphicsEllipseItem* rightTop;
    // QGraphicsEllipseItem* center;
    // QGraphicsEllipseItem* leftBottom;
    // QGraphicsEllipseItem* rightBottom;
    // QList<QGraphicsEllipseItem*> items;


private:
    Ui::MainWindow *ui;
    Qylon::vTools *vTools;
    QString version="";



};
#endif // MAINWINDOW_H
