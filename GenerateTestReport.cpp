#include "GenerateTestReport.h"
#include "ui_GenerateTestReport.h"
#include <QFileDialog>
#include <QMessageBox>
GenerateTestReport::GenerateTestReport(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GenerateTestReport)
{
    ui->setupUi(this);
    connect(ui->pushButton_Generate, &QPushButton::clicked, this, [=]{
        QString camName = ui->lineEdit_Camera->text();
        QString lensName = ui->lineEdit_Lens->text();

        if(lensName.isEmpty()){
            QMessageBox::critical(this, this->windowTitle(), "You must put the lens serial number into the box.");
            return;
        }

        QString timestamp = QDateTime::currentDateTime().toString("yy_MM_dd_hh_mm_ss");
        QString defaultFileName = QDir::homePath() + "/" + camName + "_" + lensName + "_" + timestamp + ".docx";
        QString filePath = QFileDialog::getSaveFileName(
            this,
            "Generate a Test Report",
            defaultFileName,
            "Word Documents (*.doc *.docx)"
            );
        if(filePath.isEmpty()) return;

        if(!generateFile(filePath)){
            QMessageBox::warning(this, this->windowTitle(), "Can't generate a test report properly.");
        }else
            QMessageBox::information(this, this->windowTitle(), "Generated a test report successfuly.");
    });
}

GenerateTestReport::~GenerateTestReport()
{
    delete ui;
}

void GenerateTestReport::setCameraInformation(int serial, double pixres)
{
    ui->lineEdit_Camera->setText(QString::number(serial));
    ui->label_Pixres->setText(QString::number(pixres));
}

void GenerateTestReport::setMagnification(double mag)
{
    ui->label_Mag->setText(QString::number(mag));
}

bool GenerateTestReport::generateFile(QString path)
{
    auto camSerial = ui->lineEdit_Camera->text();
    auto lensSerial = ui->lineEdit_Lens->text();
    auto pixres = ui->label_Pixres->text().toInt();


    return false;
}
