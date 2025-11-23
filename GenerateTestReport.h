#ifndef GENERATETESTREPORT_H
#define GENERATETESTREPORT_H

#include <QDialog>

namespace Ui {
class GenerateTestReport;
}

class GenerateTestReport : public QDialog
{
    Q_OBJECT

public:
    explicit GenerateTestReport(QWidget *parent = nullptr);
    ~GenerateTestReport();

    void setCameraInformation(int serial, double pixres);
    void setMagnification(double mag);
    void generateFile(QString path);

private:
    Ui::GenerateTestReport *ui;
};

#endif // GENERATETESTREPORT_H
