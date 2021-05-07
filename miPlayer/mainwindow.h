#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMainWindow>
#include <QString>
#include <QFile>
#include <memory>
#include <QTimer>
#include <QMediaPlayer>
#include <QSettings>
#include <QMimeData>
#include <QDragEnterEvent>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
    QTimer *timer;

private slots:
    void on_addButton_clicked();

    void on_openButton_clicked();

    void on_saveasButton_clicked();

    void on_deleteButton_clicked();

    void on_clearButton_clicked();

    void on_play_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSlider_2_valueChanged(int value);

    void on_List_doubleClicked(const QModelIndex &index);

    void on_volume_valueChanged(int value);

    void on_stop_clicked();

    void on_pauseButton_clicked();

    void on_nextButton_clicked();

    void on_previousButton_clicked();

    void metaDataChanged();

public slots:
  void iconActivated(QSystemTrayIcon::ActivationReason);
  void TimerSlot();
  void dropEvent(QDropEvent *e);
  void dragEnterEvent(QDragEnterEvent *e);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu;
    QMenu* createMenu();
    QMediaPlayer *player;
    QSettings *settings;
    QString currentFile;

protected:
     void closeEvent(QCloseEvent *event);

};
#endif // MAINWINDOW_H
