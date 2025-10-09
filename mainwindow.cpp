#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDockWidget>
#include <QFileDialog>
#include "Qylon/Camera/Camera.h"
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QClipboard>
#define FILE_NAME "Pixel_Cal_Data.csv"

using namespace Pylon;
using namespace Pylon::DataProcessing;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setSizeGrip(false);
    ui->graphicsView->setLogo(true);
    ui->graphicsView->setLogoImage(QImage(":/Resources/Logo.png"));
    auto cross = ui->graphicsView->getToolBarItems().at(4);
    auto setting = ui->graphicsView->getToolBarItems().at(5);
    ui->graphicsView->removeAction(cross);
    ui->graphicsView->removeAction(setting);
    connect(ui->actionSettings, &QAction::toggled, ui->settings, &QDockWidget::setVisible);
    connect(ui->settings, &QDockWidget::visibilityChanged, ui->actionSettings, &QAction::setChecked);
    connect(ui->actionStatistics, &QAction::toggled, ui->statistics, &QDockWidget::setVisible);
    connect(ui->statistics, &QDockWidget::visibilityChanged, ui->actionStatistics, &QAction::setChecked);
    connect(ui->actionAbout_Pixel_Resolution_Calculator, &QAction::triggered, this, [=]{
        auto *box = new QMessageBox(this);
        box->setWindowTitle(windowTitle());
        box->setTextFormat(Qt::RichText);
        box->setText(QString("<b>Pixel Resolution Calculator %1</b><br>"
                             "<I>Built with pylon 25.08 and Qt 6.7.2</I><br><br>"
                             "This program is designed to measure circular objects in an image and convert their pixel dimensions into millimeters.<br>"
                             "<b>Basler does not guarantee the accuracy of any results or calculations.</b><br><br>"
                             "This program is intended for internal use only to verify system environments.").arg(version));

        QPixmap px(":/Resources/Logo.png");
        if (!px.isNull()) {
            const QPixmap scaled = px.scaled(100, 100,
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
            box->setIconPixmap(scaled);
        }
        box->show();
    });
    connect(ui->actionAbout_Qt, &QAction::triggered, this, [=]{
        QMessageBox::aboutQt(this);
    });

    // LOAD RECIPE
    connect(ui->toolButtonRecipe, &QPushButton::clicked, this, [=]{
        auto url = QFileDialog::getOpenFileUrl(this, "Load a recipe", QDir::homePath(), "*.precipe");
        if(url.isValid()){
            QMessageBox *box = new QMessageBox(this);
            box->setWindowTitle(this->windowTitle());
            box->setText("Load the recipe...");
            box->setIcon(QMessageBox::Icon::Information);
            box->setStandardButtons(QMessageBox::NoButton);
            box->show();

            QTimer::singleShot(100, [=]{
                auto ok = vTools->loadRecipe(url.toLocalFile());
                box->hide();
                box->deleteLater();

                if(ok){
                    emit ui->toolButtonRefresh->clicked();
                    ui->lineEditRecipe->setText(url.toLocalFile());
                    QMessageBox::information(this, windowTitle(), tr("Recipe loaded successfully."));
                }else{
                    ui->lineEditRecipe->setText("");
                    QMessageBox::warning(this, windowTitle(), tr("Failed to load the recipe."));

                }
            });

        }
    });

    // SINGLE MODE
    connect(ui->toolButtonSingle, &QPushButton::clicked, this, [=]{
        vTools->startRecipe(1);
        ui->statusbar->showMessage("Start the recipe with a single mode.", 3000);
    });
    // LIVE MODE
    connect(ui->toolButtonContinuous, &QPushButton::toggled, this, [=](bool checked){
        if(checked){
            ui->statusbar->showMessage("Start the recipe with a live mode.");
            vTools->startRecipe();
        }else{
            ui->statusbar->showMessage("Stop the recipe.", 3000);
            vTools->stopRecipe();
        }
    });

    // REFRESH
    connect(ui->toolButtonRefresh, &QPushButton::clicked, this, [=]{
        // Test
        ui->comboBox->blockSignals(true);
        ui->comboBox->clear();
        ui->comboBox->addItems(getCameraList());
        ui->comboBox->setCurrentText(getCurrentCamera());
        ui->comboBox->blockSignals(false);
    });

    // CURRENT CAMERA CHANGED
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, [=](QString current){
        if(setCurrentCamera(current)){
            ui->statusbar->showMessage("The current camera changed. CAM:" + current, 3000);
        }
    });

    // SAVE A CURRENT RESULT AS CSV FORMAT
    ui->lineEditCsv->setText("D:/EVMS/TP/ENV");
    connect(ui->toolButtonCsv, &QPushButton::clicked, this, [=]{
        auto dir = QFileDialog::getExistingDirectory(this, "Set the save path", QDir::homePath());
        if(!dir.isEmpty()){
            ui->lineEditCsv->setText(dir);
        }
    });
    connect(ui->toolButtonExport, &QPushButton::clicked, this, [=]{
        dataCollection();
    });

    connect(ui->toolButtonCopy, &QPushButton::clicked, this, [=]{

        int tabIdx = ui->tabWidget->currentIndex();
        auto *table = qobject_cast<QTableWidget*>(ui->tabWidget->widget(tabIdx));
        if (!table) return;

        QString copiedText;

        QStringList headers;
        headers << ui->tabWidget->tabText(ui->tabWidget->currentIndex());
        for (int col = 0; col < table->columnCount(); ++col) {
            QString headerText = table->horizontalHeaderItem(col)->text();
            headerText.replace('\n', ' ');
            headers << headerText;
        }
        copiedText += headers.join('\t') + '\n';

        for (int row = 0; row < table->rowCount(); ++row) {
            QStringList rowData;
            rowData << QString::number(row + 1);
            for (int col = 0; col < table->columnCount(); ++col) {
                auto *item = table->item(row, col);
                rowData << (item ? item->text() : "");
            }
            copiedText += rowData.join('\t');
            if (row != table->rowCount() - 1)
                copiedText += '\n';
        }

        QApplication::clipboard()->setText(copiedText);
        ui->statusbar->showMessage("Copied this current sheet data.", 3000);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addVTools(Qylon::vTools *v)
{
    vTools = v;
    connect(vTools, &Qylon::vTools::finishedProcessing, this, [=]{
        ui->graphicsView->clear();
        const int tabIdx = addTableWidget(ui->comboBox->currentText());
        auto *table = qobject_cast<QTableWidget*>(ui->tabWidget->widget(tabIdx));

        table->clearContents();
        table->setRowCount(0);
        int lastRow = 0;
        int lastScoreRow = 0;

        auto result = vTools->getResult();
        auto images = result.images;
        if(images.size() >0){
            auto current = images.first().second;
            ui->graphicsView->setImage(convertPylonImageToQImage(current));
            drawOverlay(current.GetWidth(), current.GetHeight());
        }

        auto strings = result.strings;
        if(!strings.empty()){
            table->setRowCount(23);
            // Estimated
            double estPixres = 2.74/ui->doubleSpinBoxMag->value();
            for(int i=0; i< 23; ++i){
                auto c0 = new QTableWidgetItem("N/A");
                auto c1 = new QTableWidgetItem("N/A");
                auto c2 = new QTableWidgetItem("N/A");
                auto c3 = new QTableWidgetItem(QString::number(estPixres));
                auto c4 = new QTableWidgetItem("N/A");
                c0->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                c1->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                c2->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                c3->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                c4->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

                table->setItem(i, 0, c0); // Circle
                table->setItem(i, 1, c1); // Diameter
                table->setItem(i, 2, c2); // Pixres
                table->setItem(i, 3, c3); // Estimated
                table->setItem(i, 4, c4); // Error
            }

            for(auto str: strings){
                if(str.contains("Circle_px")){
                    QRegularExpression reC(R"(Circle_px(\d+)=CenterXY-Radius\[([\d\.]+),([\d\.]+),([\d\.]+)\])");
                    QRegularExpressionMatch match = reC.match(str);

                    if (match.hasMatch()) {
                        int index = match.captured(1).toInt();
                        double radius = match.captured(4).toDouble();
                        double diameter = radius * 2;
                        double convertedValue = (ui->doubleSpinBoxDiameter->value() / diameter) * 1000;
                        table->item(index-1, 1)->setText(QString::number(diameter));
                        table->item(index-1, 2)->setText(QString::number(convertedValue));
                        double error = std::abs(estPixres - convertedValue);
                        double percentageError = (error / convertedValue) * 100;
                        table->item(index-1, 4)->setText(QString::number(percentageError));
                    }
                }
                if(str.contains("Score")){
                    QRegularExpression re(R"(Score(\d+)=([+-]?\d*\.?\d+))");
                    QRegularExpressionMatch scoreMatch = re.match(str);
                    if(scoreMatch.hasMatch()){
                        int index = scoreMatch.captured(1).toInt();
                        QString scoreValue = scoreMatch.captured(2);

                        bool numeric = false;
                        double val = scoreValue.toDouble(&numeric);

                        if(numeric){
                            table->item(index-1, 0)->setText(QString::number(val));
                        }
                    }
                }
            }
        }

        auto items = result.items;
        for(int i=0; i< items.size(); ++i){
            if(items.at(i).first.contains("Region")) continue;
            auto item = items.at(i).second;
            auto ellipse = static_cast<QGraphicsEllipseItem*>(item);
            QBrush brush;
            brush.setColor(QColor(255,0,0,80));
            brush.setStyle(Qt::SolidPattern);
            ellipse->setBrush(brush);
            ellipse->setPen(Qt::NoPen);

            ui->graphicsView->addGraphicsItem(items.at(i).second);
        }
    });
}

void MainWindow::initialize()
{
    emit ui->toolButtonRefresh->clicked();
    ui->lineEditRecipe->setText(vTools->getCurrentRecipePath());
}

QStringList MainWindow::getCameraList()
{
    try{
        auto currentRecipe = vTools->getRecipe();
        QStringList cameras;

        // Update available devices
        auto deviceUpdate = currentRecipe->GetParameters().Get(CommandParameterName("Camera/@vTool/DeviceListUpdate"));
        deviceUpdate.Execute();

        // Searching devices
        CIntegerParameter deviceSelector = currentRecipe->GetParameters().Get(IntegerParameterName("Camera/@vTool/DeviceSelector"));
        int deviceCnt = deviceSelector.GetMax();

        for(int i=0; i<=deviceCnt; ++i){
            deviceSelector.TrySetValue(i);
            auto serialNum = currentRecipe->GetParameters().Get(StringParameterName("Camera/@vTool/DeviceSerialNumber")).ToString();
            qDebug() << "Found a device. " << serialNum;
            cameras.push_back(serialNum.c_str());
        }
        ui->statusbar->showMessage("Found " + QString::number(cameras.count()) +" Camera(s).");
        return cameras;

    }catch(const Pylon::GenericException &e){
        qDebug() << e.what();
    }

    return QStringList();
}

QString MainWindow::getCurrentCamera()
{
    try{
        auto currentRecipe = vTools->getRecipe();
        auto selected = currentRecipe->GetParameters().Get(StringParameterName("Camera/@vTool/SelectedDeviceSerialNumber"));

        return selected.GetValue().c_str();
    }catch(const Pylon::GenericException &e){
        qDebug() << e.what();
    }
    return QString();
}

bool MainWindow::setCurrentCamera(QString cameraSerial)
{
    try{
        auto currentRecipe = vTools->getRecipe();
        currentRecipe->DeallocateResources();

        // Searching devices
        CIntegerParameter deviceSelector = currentRecipe->GetParameters().Get(IntegerParameterName("Camera/@vTool/DeviceSelector"));
        int deviceCnt = deviceSelector.GetMax();

        for(int i=0; i<=deviceCnt; ++i){
            deviceSelector.TrySetValue(i);
            auto serialNum = currentRecipe->GetParameters().Get(StringParameterName("Camera/@vTool/DeviceSerialNumber")).ToString();
            if(serialNum == cameraSerial){
                qDebug() << "Found a device. " << serialNum << cameraSerial;
                CCommandParameter deviceChoose = currentRecipe->GetParameters().Get(CommandParameterName("Camera/@vTool/DeviceChoose"));
                deviceChoose.Execute();

                currentRecipe->PreAllocateResources();
                return true;
            }
        }
    }catch(const Pylon::GenericException& e){
        qDebug() << e.what();
    }
    return false;
}

int MainWindow::addTableWidget(QString serialNum)
{
    for (int i = 0; i < ui->tabWidget->tabBar()->count(); ++i) {
        if (ui->tabWidget->tabText(i) == serialNum) {
            ui->tabWidget->tabBar()->setCurrentIndex(i);
            return i;
        }
    }

    auto *table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Circle\nScore (%)", "Ø (px)", "Pixel Res.\n(μm/px)", "Estimated\n(μm/px)", "Error (%)"});

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Ø(px)
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Pix-Res
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Score
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Estimated
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Error


    table->verticalHeader()->setVisible(true);
    table->verticalHeader()->setDefaultSectionSize(22);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    table->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
#endif

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    int idx = ui->tabWidget->addTab(table, serialNum);
    ui->tabWidget->tabBar()->setCurrentIndex(idx);
    return idx;
}

QStringList MainWindow::dataCollection()
{
    QStringList csvLines;
    csvLines << "Camera_Serial_Num,Pixel_Resolution(um/px)"; // Header

    for(int i=0; i< ui->tabWidget->tabBar()->count(); ++i){
        auto *table = qobject_cast<QTableWidget*>(ui->tabWidget->widget(i));

        qDebug() << "Table:" << ui->tabWidget->tabText(i);
        QList<double> pixresList;
        for(int j=0; j < table->rowCount(); ++j){
            auto data = table->item(j, 2)->text();
            if(data.isEmpty() || data == "N/A") continue;

            pixresList.push_back(table->item(j, 2)->text().toDouble());
        }

        double avg = 0.0;
        if (!pixresList.isEmpty()) {
            double sum = std::accumulate(pixresList.begin(), pixresList.end(), 0.0);
            avg = sum / pixresList.size();
        }

        QString serialNum = ui->tabWidget->tabText(i);
        csvLines << QString("%1,%2").arg(serialNum).arg(QString::number(avg, 'f', 3));
    }


    QString dirPath = ui->lineEditCsv->text();
    QString fileName = QString::fromStdString(FILE_NAME);

    QDir dir(dirPath);
    QString savePath = dir.filePath(fileName);

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString &line : csvLines)
            out << line << "\n";
        file.close();
        QMessageBox::information(this, this->windowTitle(), "Successfully save the csv file.\n" + savePath);
    } else {
        qDebug() << "CSV Failed:" << savePath;
        QMessageBox::warning(this, this->windowTitle(), "Failed to save the csv file.\n" + savePath);
    }

    return QStringList();
}

void MainWindow::drawOverlay(int width, int height)
{

    auto leftTop = new QGraphicsEllipseItem;
    auto rightTop = new QGraphicsEllipseItem;
    auto center = new QGraphicsEllipseItem;
    auto leftBottom = new QGraphicsEllipseItem;
    auto rightBottom = new QGraphicsEllipseItem;
    QList<QGraphicsEllipseItem*> items;
    items << leftTop << rightTop << center << leftBottom << rightBottom;

    QBrush brush;
    brush.setColor(QColor(0,255,0,175));
    brush.setStyle(Qt::SolidPattern);

    for(auto cur : items){
        cur->setBrush(brush);
        cur->setPen(Qt::NoPen);
    }

    double c_width = 700;
    double c_height = 700;
    double distance = 878;
    double x = (width / 2) - (c_width/2);
    double y = (height / 2) - (c_height/2);

    leftTop->setRect(x - distance*2, y - distance*2, c_width,c_height);
    rightTop->setRect(x + distance*2, y - distance*2, c_width,c_height);
    center->setRect(x, y, c_width, c_height);
    leftBottom->setRect(x - distance*2, y + distance*2, c_width,c_height);
    rightBottom->setRect(x + distance*2, y + distance*2, c_width,c_height);

    ui->graphicsView->addGraphicsItem(static_cast<QGraphicsItem*>(leftTop));
    ui->graphicsView->addGraphicsItem(static_cast<QGraphicsItem*>(rightTop));
    ui->graphicsView->addGraphicsItem(static_cast<QGraphicsItem*>(center));
    ui->graphicsView->addGraphicsItem(static_cast<QGraphicsItem*>(leftBottom));
    ui->graphicsView->addGraphicsItem(static_cast<QGraphicsItem*>(rightBottom));
}

void MainWindow::builderRecipe()
{
    Pylon::DataProcessing::CBuildersRecipe recipe;
    recipe.AddOutput("Image", Pylon::DataProcessing::VariantDataType_PylonImage);
    recipe.AddVTool("Camera", "846BCA11-6BF2-4895-88C4-FE038F5A659C");
    recipe.AddConnection("camera_to_output",
                         "Camera.Image",
                         "<RecipeOutput>.Image");


    int circleMeasurementCount = 23;
    for(int i=1; i<=circleMeasurementCount; ++i ){
        QString name = "CircleMeasurementsPro" + QString::number(i);
        recipe.AddVTool(name.toStdString().c_str(), "4AD86113-A1A6-46A1-BC88-E5C8DB6782EA");

        recipe.AddConnection(QString("camera_to_circle_" + QString::number(i)).toStdString().c_str(),
                             "Camera.Image",
                             QString(name + ".Image").toStdString().c_str());

        recipe.AddOutput(QString("Circle_px"+ QString::number(i)).toStdString().c_str(),
                         Pylon::DataProcessing::VariantDataType_CircleF);
        recipe.AddConnection(QString("circle_" + QString::number(i) + "_to_output").toStdString().c_str(),
                             (name + ".Circle_px").toStdString().c_str(),
                             QString("<RecipeOutput>.Circle_px" + QString::number(i)).toStdString().c_str());

        recipe.AddOutput(QString("Score" + QString::number(i)).toStdString().c_str(),
                         Pylon::DataProcessing::VariantDataType_Float);
        recipe.AddConnection(QString("Score_" + QString::number(i) + "_to_output").toStdString().c_str(),
                             (name + ".Score").toStdString().c_str(),
                             QString("<RecipeOutput>.Score" + QString::number(i)).toStdString().c_str());

        auto recipeName = name + "/@vTool/";
        auto val = recipe.GetParameters().Get(IntegerParameterName((recipeName + "Column").toStdString().c_str()));

        recipe.GetParameters().GetAllParameterNames();
        qDebug() << val.GetValue();
    }
    recipe.Start(EAcquisitionMode::AcquisitionMode_SingleFrame);
    recipe.Stop();

    recipe.SaveAs(ERecipeFileFormat::RecipeFileFormat_JsonDefault, "C:/Users/minwoo/Downloads/test.precipe");
    vTools->loadRecipe("C:/Users/minwoo/Downloads/test.precipe");
}

void MainWindow::checkRecipe()
{
    // int circleMeasurementCount = 23;
    // int sampleSegmentValue = 500;
    // double sampleSegmentWidthValue = 4.1;
    // double sampleToleValue = 5.0;
    // int shapeTolValue = 150;

    // auto newRecipe = vTools->getRecipe();
    // for(int i=1; i<=circleMeasurementCount; ++i){
    //     QString name = "CircleMeasurementsPro" + QString::number(i);
    //     auto recipeName = name + "/@vTool/";

    //     try{
    //         auto col = newRecipe->GetParameters().Get(IntegerParameterName((recipeName + "Column").toStdString().c_str()));
    //         auto row = newRecipe->GetParameters().Get(IntegerParameterName((recipeName + "Row").toStdString().c_str()));
    //         // The length from outer to inner circle
    //         auto shapeTor = newRecipe->GetParameters().Get(FloatParameterName((recipeName + "ShapeTolerance").toStdString().c_str()));

    //         auto sample = newRecipe->GetParameters().Get(IntegerParameterName((recipeName + "NumberSampleSegments").toStdString().c_str()));
    //         auto sampleWidth = newRecipe->GetParameters().Get(FloatParameterName((recipeName + "SampleSegmentWidth").toStdString().c_str()));
    //         auto sampleTor = newRecipe->GetParameters().Get(FloatParameterName((recipeName + "SampleTolerance").toStdString().c_str()));
    //         qDebug() << name
    //                  <<"\nShape Tole:" << shapeTor.GetValue()
    //                  << "\nSampleSegments:" << sample.GetValue()
    //                  << "\nSampleWidth:" << sampleWidth.GetValue()
    //                  << "\nSampleTor:" << sampleTor.GetValue() ;
    //         qDebug() << "Edited:" << sample.TrySetValue(sampleSegmentValue) << sampleWidth.TrySetValue(sampleSegmentWidthValue)
    //                  << sampleTor.TrySetValue(sampleToleValue) << shapeTor.TrySetValue(shapeTolValue);
    //         qDebug() << "\n\n";

    //     }catch(const GenericException &e){ qDebug()<< e.what();}
    // }

    try{
        auto recipe = vTools->getRecipe();
        auto params = recipe->GetParameters().GetAllParameterNames();

        for(auto par: params){
            auto var = recipe->GetParameters().Get(par);
            if(var.IsValid()){
                auto str1 = var.GetInfo(EParameterInfo::ParameterInfo_Name);
                auto str2 = var.GetInfo(EParameterInfo::ParameterInfo_DisplayName);
                auto str3 = var.GetInfo(EParameterInfo::ParameterInfo_ToolTip);

                qDebug() << str1 << "|" << str2 <<  "\n" << str3;
            }
        }
    }catch(const GenericException &e){
        qDebug()<< e.what();
    }
}

void MainWindow::setVersion(QString vers)
{
    version = vers;
}
