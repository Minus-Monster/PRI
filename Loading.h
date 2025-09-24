#ifndef LOADING_H
#define LOADING_H

#include <QWidget>

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
};

#endif // LOADING_H
