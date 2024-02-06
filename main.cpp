#include "mainwindow.h"
//#include "dragprogressbar.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QTime>
#include <QDate>
#include <QDomDocument>
QTranslator translator;
void writeLog(const QString& msg,const QString& filePath,const int& lineNumber)
{
    if(!QDir::current().exists("Log"))
    {
        QDir::current().mkdir("Log");
    }
    QString nowDate = QDate::currentDate().toString("yyyy-MM-dd");
    QString nowTime = QTime::currentTime().toString("HH:mm:ss");
    QFile file(QString("./Log/%1error.txt").arg(nowDate));
    if(file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        QString complete = QString("%1 %2%3 msg:%4\n").arg(nowTime).arg(filePath).arg(lineNumber).arg(msg);
        file.write(complete.toUtf8());
        file.close();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("fusion");
    bool status;
    QFile file("./config.xml");
    if(file.open(QIODevice::ReadOnly))
    {
        QDomDocument document("config");
        QString errorMsg;
        int errorRow;
        int errorCol;
        if(document.setContent(&file,&errorMsg,&errorRow,&errorCol))
        {
            QString language = document.elementsByTagName("language").at(0).firstChild().nodeValue();
            if(language == "en_US")
            {
                bool ok = translator.load(":/i18n/AudioPlayerback_en.qm");
                if(ok)
                {
                    a.installTranslator(&translator);
                    status = true;
                }
                else
                {
                    writeLog("加载:/i18n/AudioPlayerback_en.qm失败",__FILE__,__LINE__);
                    return 1;
                }
            }
            else if(language == "zh_CN")
            {
                bool ok = translator.load(":/i18n/AudioPlayerback_zh_CN.qm");
                if(ok)
                {
                    a.installTranslator(&translator);
                    status = false;
                }
                else
                {
                    writeLog("加载:/i18n/AudioPlayerback_zh_CN.qm失败",__FILE__,__LINE__);
                    return 1;
                }
            }
            else
            {
                writeLog("./config.xml中的language值不对",__FILE__,__LINE__);
                return 1;
            }
        }
        else
        {
            QString error = QString("config xml read error:%1").arg(errorMsg);
            writeLog(error,__FILE__,__LINE__);
            return 1;
        }
    }
    else
    {
        qDebug()<<"./config.xml no open fail,error:"<<file.errorString();
        writeLog(QString("./config.xml no open fail,error:%1").arg(file.errorString()),__FILE__,__LINE__);
        return 1;
    }
    MainWindow w;
    qDebug()<<"status:"<<status;
    w.setActionLanguageStatus(status);
    w.show();
    return a.exec();
}
