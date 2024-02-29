#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <Windows.h>
#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QWidgetAction>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QActionGroup>
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
#include <QStandardPaths>
#include <QMessageBox>
#include <iostream>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QDomDocumentType>
#include <QFileSystemWatcher>
#include <QString>
#include <QTranslator>
#include <QDomNode>
#include <QReadWriteLock>
#include <QMutex>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

extern void writeLog(const QString& msg,const QString& filePath,const int& lineNumber);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setActionLanguageStatus(bool status);
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
    void do_playerErrorOccurred(QMediaPlayer::Error error, const QString &errorString);
    void do_configFileChanged(const QString& filePath);
    void on_actionEnglish_triggered(bool checked);
    void do_metaDataChanged();
    void on_lineEditSearch_editingFinished();
    void on_actionChinese_triggered(bool checked);

private:
    void initLog();
    void readConfig();
    void languageChanged(QString lang);
    void addRow(const QString& path);
    void initRecentMenu();
    void initConfig();
    Ui::MainWindow *ui;
    QStringList headers;
    QStringList headerLanguage;
    QDoubleSpinBox* volumeSpix;
    QDoubleSpinBox* rateSpin;
    QWidgetAction* widgetAction;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;
    QStandardItemModel* dataModel;
    QItemSelectionModel* itemSelectionModel;
    QMenu* menuRecent;
    QLabel* labelDuraction;
    QLabel* labelPosition;
    QLabel* labelPath;
    QStringList paths;
    QString musicPath;
    QString audioFileFormats;
    QString language;//中文zh_CN,英文en_US
    QFileSystemWatcher* fileSystemWatcher;
    QFile configFile;
    QActionGroup* actionGroup;
};
#endif // MAINWINDOW_H
