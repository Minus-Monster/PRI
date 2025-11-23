#include "GenerateTestReport.h"
#include "ui_GenerateTestReport.h"

GenerateTestReport::GenerateTestReport(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GenerateTestReport)
{
    ui->setupUi(this);
}

GenerateTestReport::~GenerateTestReport()
{
    delete ui;
}

void GenerateTestReport::setCameraInformation(int serial, double pixres)
{
    ui->lineEdit_Camera->setText(QString::number(serial));
    ui->label_Pixres->setText("Pixel Resolution: " + QString::number(pixres));
}

void GenerateTestReport::setMagnification(double mag)
{
    ui->label_Mag->setText("Magnification: " + QString::number(mag));
}

void GenerateTestReport::generateFile(QString path)
{

}
