#include "Loading.h"
#include "ui_Loading.h"

Loading::Loading(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Loading)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::ApplicationModal);
    ui->progressBar->setHidden(true);
    ui->message->setHidden(true);
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
    ui->version->setText(version);
}


void Loading::update(QString message, int progress)
{
    ui->progressBar->setHidden(false);
    ui->message->setHidden(false);
    ui->message->setText(message);
    ui->progressBar->setValue(progress);
}

