#ifndef LOADING_H
#define LOADING_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class Loading;
}

class Loading : public QWidget
{
    Q_OBJECT

public:
    explicit Loading(QWidget *parent = nullptr, QString name="", QString version="");
    ~Loading();

    void setName(QString name);
    void setVersion(QString version);
    void update(QString message, int progress);

private:
    Ui::Loading *ui;
    QTimer* colorTimer;
    int colorIndex = 0;
};

#endif // LOADING_H
