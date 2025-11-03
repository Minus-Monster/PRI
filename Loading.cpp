#include "Loading.h"
#include "ui_Loading.h"
Loading::Loading(QWidget *parent, QString name, QString version)
    : QWidget(parent)
    , ui(new Ui::Loading)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::ApplicationModal);

    if(!name.isEmpty()) setName(name);
    if(!version.isEmpty()) setVersion(version);

    ui->progressBar->setHidden(true);
    ui->message->setHidden(true);

    setStyleSheet(R"(
        QWidget#Loading{
            background-color:rgb(20, 72, 126);
            color:white;
        }
        QLabel{
            color:white;
        }
    )");

    QVector<QString> colorList = {
        "#ffffff", "#fef8f1", "#fef1e3", "#fdebd6", "#fde4c8", "#fddebb", "#fcd7ad", "#fcd0a0",
        "#fbca92", "#fbc385", "#fbbd77", "#fab66a", "#faaf5c", "#f9a94f", "#f9a241", "#f99c34",
        "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34",
        "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34",
        "#f99c34", "#f99c34", "#f9a241", "#f9a94f", "#faaf5c", "#fab66a", "#fbbd77", "#fbc385",
        "#fbca92", "#fcd0a0", "#fcd7ad", "#fddebb", "#fde4c8", "#fdebd6", "#fef1e3", "#fef8f1", "#ffffff"
    };


    colorTimer = new QTimer(this);
    connect(colorTimer, &QTimer::timeout, this, [this, colorList](){
        ui->progressBar->setStyleSheet(QString(R"(
            QProgressBar{
                background-color: transparent;
                color:white;
                min-height: 3 px;
                max-height: 3 px;
            }
            QProgressBar::chunk{
                border: 3px;
                border-radius: 8px;
                background: %1;
            }
        )").arg(colorList[colorIndex]));
        colorIndex = (colorIndex + 1) % colorList.size();
    });
    colorTimer->start(50);


}

Loading::~Loading()
{
    delete ui;
}

void Loading::setName(QString name)
{
    ui->title->setText(name);
}

void Loading::setVersion(QString version)
{
    ui->version->setText("Version: " + version);
}


void Loading::update(QString message, int progress)
{
    ui->progressBar->setHidden(false);
    ui->message->setHidden(false);
    ui->message->setText(message);
    ui->progressBar->setValue(progress);
}

