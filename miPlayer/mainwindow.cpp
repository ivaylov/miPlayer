#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addnewdialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <memory>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QCloseEvent>
#include <QMediaPlayer>
#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoProbe>
#include <QAudioProbe>
#include <QMediaMetaData>
#include <QtWidgets>
#include <QMimeData>
#include <QDragEnterEvent>


static int slidervalue;
static int sliderdelay = 10000;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    trayIcon = new QSystemTrayIcon(this);
    auto menu = this->createMenu();
    trayIcon->setContextMenu(menu);
    trayIcon->setIcon(QIcon::fromTheme("media-playback-start"));
    trayIcon->show();
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);

    timer = new QTimer(this);

    player = new QMediaPlayer(this);
    settings = new QSettings("settings.txt",QSettings::IniFormat,this);


    connect(timer, SIGNAL(timeout()),this, SLOT(TimerSlot()));

    connect(player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &MainWindow::metaDataChanged);


    slidervalue = ui->horizontalSlider->value();
    int valslidmin = slidervalue * 1000 * 60;
    timer->start(valslidmin);

    int intvalue = ui->horizontalSlider->value();
    QString stringvalue = QString::number(intvalue);
    QString textmin = "Minutes: " + QString::number( intvalue );
    ui->labelMinutes->setText(textmin.toStdString().c_str());

    int intdelay = ui->horizontalSlider_2->value();
    QString stringdelay = QString::number(intdelay);
    QString textdelay = "Milliseconds: " + QString::number( intdelay );
    ui->labelDelay->setText(textdelay.toStdString().c_str());

    int vol = ui->volume->value();
    QString volstring = "volume: " + QString::number( vol );
    ui->label_6->setText(volstring.toStdString().c_str());

    QString fileName = "List.txt";
    QFile file(fileName);
    currentFile = fileName;
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {

    QSystemTrayIcon* ic = new QSystemTrayIcon(this);
    ic->setVisible(true);
    ic->showMessage(tr("miPlayer - open file"), tr(file.errorString().toStdString().c_str()), QSystemTrayIcon::Information, sliderdelay);
    ic->setVisible(false);
    qApp->processEvents();

    return;
    }
    QTextStream stream(&file);
    QString buffer;

    while (!stream.atEnd())
    {
    buffer = stream.readLine(256);
    ui->List->addItem(buffer);
    }
    file.close();


    player->setVolume(ui->volume->value());
    int volumeint = settings->value("volume").toInt();
    ui->volume->setValue(volumeint);

    bool autonextstationcheck = settings->value("autonextstationcheck").toBool();
    ui->checkBox->setChecked(autonextstationcheck);

    int autonextstationminutes = settings->value("autonextstationminutes").toInt();
    ui->horizontalSlider->setValue(autonextstationminutes);

    int notifydelay = settings->value("notifydelay").toInt();
    ui->horizontalSlider_2->setValue(notifydelay);

    setAcceptDrops(true);

}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        ui->List->addItem(fileName);
    }
}

QMenu* MainWindow::createMenu()
{
  // App can exit via Quit menu
  auto quitAction = new QAction("&Quit", this);
  connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

  auto menu = new QMenu(this);
  menu->addAction(quitAction);

  return menu;
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason_)
{
  switch (reason_) {
  case QSystemTrayIcon::Trigger:
    this->setVisible(!isVisible());
    break;
  default:
    ;
  }
}
void MainWindow::metaDataChanged()
{
    if (player->metaData(QMediaMetaData::Title).toString() != nullptr) {
        ui->info->setText(player->metaData(QMediaMetaData::Title).toString());
        setWindowTitle("miPlayer - "+player->metaData(QMediaMetaData::Title).toString());
        trayIcon->setToolTip(player->metaData(QMediaMetaData::Title).toString());
    }
    else{
        setWindowTitle("miPlayer - playlist");
        trayIcon->setToolTip(ui->List->currentItem()->text());

    }
}
void MainWindow::closeEvent(QCloseEvent *)
{

    QString fileName = "List.txt";
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {

        QSystemTrayIcon* ic = new QSystemTrayIcon(this);
        ic->setVisible(true);
        ic->showMessage(tr("miPlayer - save file"), tr(file.errorString().toStdString().c_str()), QSystemTrayIcon::Information, sliderdelay);
        ic->setVisible(false);
        qApp->processEvents();

        return;
    }
    currentFile = fileName;
    QTextStream out(&file);
    int listLength = ui->List->count();
    for (int i = 0; i < listLength; ++i)
    {
        out << ui->List->item(i)->text() << '\n';
    }
    file.close();

    timer->stop();
    timer->destroyed();
    QApplication::quit();

    settings->setValue("volume",ui->volume->value());
    settings->setValue("autonextstationcheck",ui->checkBox->isChecked());
    settings->setValue("autonextstationminutes",ui->horizontalSlider->value());
    settings->setValue("notifydelay",ui->horizontalSlider_2->value());

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addButton_clicked()
{
    QString todoDesc;
    auto dialog = std::make_unique<AddNewDialog>(todoDesc);
    dialog->show();
    dialog->exec();
    ui->List->addItem(todoDesc);
}

void MainWindow::on_openButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open the file");
    QFile file(fileName);
    currentFile = fileName;
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {

        return;
    }
    QTextStream stream(&file);
    QString buffer;

    while (!stream.atEnd())
    {
        buffer = stream.readLine(256);
        ui->List->addItem(buffer);
    }
    file.close();

}

void MainWindow::on_saveasButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save as");
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {


        QSystemTrayIcon* ic = new QSystemTrayIcon(this);
        ic->setVisible(true);
        ic->showMessage(tr("miPlayer - save file"), tr(file.errorString().toStdString().c_str()), QSystemTrayIcon::Information, sliderdelay);
        ic->setVisible(false);
        qApp->processEvents();

        return;
    }
    currentFile = fileName;
    QTextStream out(&file);
    int listLength = ui->List->count();
    for (int i = 0; i < listLength; ++i)
    {
        out << ui->List->item(i)->text() << '\n';
    }
    file.close();
}

void MainWindow::on_deleteButton_clicked()
{
    if (ui->List->currentItem() == nullptr)
    {
        qDebug() << "Tried to delete with no item selected, bug!";
        return;
    }

    auto takenItem = ui->List->takeItem(ui->List->currentRow());
    delete takenItem;
}

void MainWindow::on_clearButton_clicked()
{
    ui->List->clear();
}

void MainWindow::on_play_clicked()
{

    if(ui->List->currentItem()){

        player->stop();
        player->setMedia(QUrl::fromUserInput(ui->List->currentItem()->text()));
        player->setVolume(ui->volume->value());
        player->play();

        ui->info->setText(ui->List->currentItem()->text());

    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    int intvalue = value;
    QString stringvalue = QString::number(intvalue);
    QString text = "Minutes: " + QString::number( intvalue );
    ui->labelMinutes->setText(text.toStdString().c_str());

    slidervalue = ui->horizontalSlider->value();
    int valslidmin = slidervalue * 1000 * 60;
    timer->setInterval(valslidmin);
    slidervalue = value;

}


void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    int intdelay = ui->horizontalSlider_2->value();
    QString stringdelay = QString::number(intdelay);
    QString textdelay = "Milliseconds: " + QString::number( intdelay );
    ui->labelDelay->setText(textdelay.toStdString().c_str());

    sliderdelay = ui->horizontalSlider->value();
    sliderdelay = value;
}

void MainWindow::TimerSlot(){

    slidervalue = ui->horizontalSlider->value();
    int valslidmin = slidervalue * 1000 * 60;
    timer->setInterval(valslidmin);
    if(ui->checkBox->isChecked())
    {
        int currentIndex = ui->List->currentRow();
        ui->List->setCurrentRow(currentIndex+1);

        if(ui->List->currentItem()){
            QString str = ui->List->currentItem()->text();

            player->stop();
            player->setMedia(QUrl::fromUserInput(ui->List->currentItem()->text()));
            player->setVolume(ui->volume->value());
            player->play();

            ui->info->setText(ui->List->currentItem()->text());
        }
    }
}


void MainWindow::on_List_doubleClicked(const QModelIndex &)
{
    player->stop();
    player->setMedia(QUrl::fromUserInput(ui->List->currentItem()->text()));
    player->setVolume(ui->volume->value());
    player->play();

    ui->info->setText(ui->List->currentItem()->text());
}

void MainWindow::on_volume_valueChanged(int value)
{
    player->setVolume(value);

    int vol = ui->volume->value();
    QString volume = "volume: " + QString::number( vol );
    ui->label_6->setText(volume.toStdString().c_str());

}

void MainWindow::on_stop_clicked()
{
    player->stop();
    ui->info->setText("miPlayer");
    setWindowTitle("miPlayer - playlist");
}

void MainWindow::on_pauseButton_clicked()
{
    player->pause();
    ui->info->setText("miPlayer");
    setWindowTitle("miPlayer - playlist");

}

void MainWindow::on_nextButton_clicked()
{
    int currentIndex = ui->List->currentRow();
    ui->List->setCurrentRow(currentIndex+1);
    ui->info->setText(ui->List->currentItem()->text());

    player->stop();
    player->setMedia(QUrl::fromUserInput(ui->List->currentItem()->text()));
    player->setVolume(ui->volume->value());
    player->play();
}

void MainWindow::on_previousButton_clicked()
{
    int currentIndex = ui->List->currentRow();
    ui->List->setCurrentRow(currentIndex-1);
    ui->info->setText(ui->List->currentItem()->text());

    player->stop();
    player->setMedia(QUrl::fromUserInput(ui->List->currentItem()->text()));
    player->setVolume(ui->volume->value());
    player->play();
}
