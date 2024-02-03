#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QWidgetAction>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaMetaData>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QFontDatabase>
#include <QModelIndex>
#include <QImage>
#include <QDir>
#include <QMediaFormat>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QCursor>
#include <QClipboard>
#include <QMessageBox>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);
    void closeEvent(QCloseEvent* e);
private slots:
    void on_progressBar_valueChanged(int value);
    void do_contextMenuRequested(const QPoint& pos);
    void on_btnStart_clicked();
    void do_actionCopyTriggered();
    void do_actionRemoveTriggered();
    void do_actionClearTriggered();
    void on_btnPause_clicked();
    void do_stateChanged(QMediaPlayer::PlaybackState state);
    void do_tableViewDoubleClicked();
    void on_actionOpenFile_triggered();
    void do_itemSelectionRowChanged(const QModelIndex& current,const QModelIndex& previous);
    void on_btnStop_clicked();
    void do_spinValueChanged(double value);
    void do_playerPositionChanged(qint64 position);
    void do_playerDuractionChanged(qint64 duraction);
    void on_actionOpenDir_triggered();
    void do_playerPlaybackRateChanged(double rate);
    void do_playerSourceChanged(const QUrl& url);
    void do_menuTriggered(QAction* action);
    void do_playerBufferProgressChanged(float value);
    void do_playerSeekableChanged(bool status);
private:
    void addRow(const QString& path);
    void initRecentMenu();
    Ui::MainWindow *ui;
    QDoubleSpinBox* volumeSpix;
    QDoubleSpinBox* rateSpin;
    QWidgetAction* widgetAction;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;
    QStandardItemModel* dataModel;
    QItemSelectionModel* itemSelectionModel;
    QMediaFormat mediaFormat;
    QMenu* menuRecent;
    QLabel* labelDuraction;
    QLabel* labelPosition;
    QLabel* labelPath;
    QStringList paths;

};
#endif // MAINWINDOW_H
