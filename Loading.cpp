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
        "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34",
        "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34",
        "#f9a84d", "#fab466", "#fbc180", "#fccd99", "#fcd9b2", "#fde6cc", "#fef2e5", "#ffffff",
        "#ffffff", "#fef2e5", "#fde6cc", "#fcd9b2", "#fccd99", "#fbc180", "#fab466", "#f9a84d",
        "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34",
        "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34", "#f99c34"
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
    colorTimer->start(100);


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

