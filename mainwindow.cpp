#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);
    actionGroup->addAction(ui->actionChinese);
    actionGroup->addAction(ui->actionEnglish);
    this->setAcceptDrops(true);
    this->setWindowTitle(tr("音频播放器"));
    headers<<"文件名"<<"标题"<<"作者"<<"作曲家"<<"时间";
    ui->tableView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->tableView->setAcceptDrops(false);
    rateSpin = new QDoubleSpinBox(this);
    rateSpin->setRange(0,30);
    rateSpin->setSingleStep(0.25);
    rateSpin->setSuffix(tr("倍速"));
    labelPath = new QLabel(this);
    labelPath->setText(tr("文件路径:"));
    labelDuraction = new QLabel(this);
    labelPosition = new QLabel(this);
    ui->statusbar->addWidget(labelPath,4);
    ui->statusbar->addWidget(labelPosition,1);
    ui->statusbar->addWidget(labelDuraction,1);
    volumeSpix = new QDoubleSpinBox(this);
    volumeSpix->setRange(0.00,1.00);
    volumeSpix->setSingleStep(0.01);
    volumeSpix->setSuffix(tr("音量"));
    connect(volumeSpix,&QDoubleSpinBox::valueChanged,this,&MainWindow::do_spinValueChanged);
    widgetAction = new QWidgetAction(this);
    QWidgetAction* widgetRateAction = new QWidgetAction(this);
    widgetRateAction->setDefaultWidget(rateSpin);
    widgetAction->setDefaultWidget(volumeSpix);
    ui->menubar->addAction(ui->actionOpenFile);
    ui->menubar->addAction(ui->actionOpenDir);
    menuRecent = new QMenu(this);
    menuRecent->setTitle(tr("最近打开的文件"));
    connect(menuRecent,&QMenu::triggered,this,&MainWindow::do_menuTriggered);
    ui->menubar->addMenu(menuRecent);
    ui->toolBar->addAction(widgetAction);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(widgetRateAction);
    ui->toolBar->addAction(ui->actionEnglish);
    ui->toolBar->addAction(ui->actionChinese);
    player = new QMediaPlayer(this);
    connect(rateSpin,&QDoubleSpinBox::valueChanged,this,&MainWindow::do_playerPlaybackRateChanged);
    QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
    audioOutput = new QAudioOutput(audioDevice,this);
    float volume = audioOutput->volume();
    rateSpin->setValue(player->playbackRate());
    volumeSpix->setValue(volume);
    player->setAudioOutput(audioOutput);
    dataModel = new QStandardItemModel(this);
    dataModel->setColumnCount(5);
    for(int i = 0;i<5;i++)
    {
        QStandardItem* item = new QStandardItem();
        item->setText(headers.at(i));
        item->setTextAlignment(Qt::AlignCenter);
        item->setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
        item->setForeground(QBrush(QGradient(QGradient::Preset::BigMango)));
        dataModel->setHorizontalHeaderItem(i,item);
    }
    configFile.setFileName("./config.xml");
    itemSelectionModel = new QItemSelectionModel(dataModel,this);
    ui->tableView->setModel(dataModel);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setSelectionModel(itemSelectionModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setResizeContentsPrecision(0);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    connect(player,&QMediaPlayer::playbackStateChanged,this,&MainWindow::do_stateChanged);
    connect(itemSelectionModel,&QItemSelectionModel::currentRowChanged,this,&MainWindow::do_itemSelectionRowChanged);
    connect(player,&QMediaPlayer::positionChanged,this,&MainWindow::do_playerPositionChanged);
    connect(player,&QMediaPlayer::durationChanged,this,&MainWindow::do_playerDuractionChanged);
    connect(player,&QMediaPlayer::sourceChanged,this,&MainWindow::do_playerSourceChanged);
    connect(player,&QMediaPlayer::bufferProgressChanged,this,&MainWindow::do_playerBufferProgressChanged);
    connect(ui->tableView,&QTableView::customContextMenuRequested,this,&MainWindow::do_contextMenuRequested);
    connect(player,&QMediaPlayer::seekableChanged,this,&MainWindow::do_playerSeekableChanged);
    connect(player,&QMediaPlayer::errorOccurred,this,&MainWindow::do_playerErrorOccurred);
    fileSystemWatcher = new QFileSystemWatcher(this);
    fileSystemWatcher->addPath(configFile.fileName());
    connect(fileSystemWatcher,&QFileSystemWatcher::fileChanged,this,&MainWindow::do_configFileChanged);
    initRecentMenu();
    initConfig();
}

void MainWindow::initLog()
{
    QDir dirLog("Log");
    if(!dirLog.exists())
    {
        QDir::current().mkdir("Log");
    }
}

void MainWindow::do_configFileChanged(const QString& filePath)
{
    QFile fileChanged(filePath);
    if(fileChanged.exists())
    {
        this->readConfig();
    }
    else
    {
        qDebug()<<filePath<<" not exists";
        writeLog(QString("%1 not exists").arg(filePath),__FILE__,__LINE__);
    }
}

void MainWindow::initConfig()
{
    if(!configFile.exists())
    {
        if(configFile.open(QIODevice::ReadWrite|QIODevice::Text))
        {
            QDomDocument* domDocument = new QDomDocument;
            QDomProcessingInstruction processInstruction = domDocument->createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
            domDocument->appendChild(processInstruction);
            QDomElement rootElement = domDocument->createElement("Document");
            domDocument->appendChild(rootElement);
            QDomElement musicPathElement = domDocument->createElement("musicPath");
            rootElement.appendChild(musicPathElement);
            QDomText musicPathNode = domDocument->createTextNode(musicPath);
            musicPathElement.appendChild(musicPathNode);
            QDomComment languageComment = domDocument->createComment("Chinese choose zh_CN, English choose en_US");
            rootElement.appendChild(languageComment);
            QDomElement languageElement = domDocument->createElement("language");
            rootElement.appendChild(languageElement);
            QDomText languageNode = domDocument->createTextNode("zh_CN");
            languageElement.appendChild(languageNode);
            QDomElement audioFileFormatsElement = domDocument->createElement("audioFileFormats");
            QDomComment fileFormatComment = domDocument->createComment("File formats are separated by spaces");
            rootElement.appendChild(fileFormatComment);
            rootElement.appendChild(audioFileFormatsElement);
            audioFileFormats = QString("*.mp3 *.flac *.webm *.aac *.quicktime *.mpeg4audio");
            QDomText fileFormatsNode = domDocument->createTextNode(audioFileFormats);
            audioFileFormatsElement.appendChild(fileFormatsNode);
            QByteArray xmlData = domDocument->toByteArray(4);
            delete domDocument;
            configFile.write(xmlData);
            configFile.close();
        }
    }
    else
        this->readConfig();
}

void MainWindow::readConfig()
{
    /**
     * QDomDocument::elementById 功能无法实现
    */
    if(configFile.exists())
    {
        if(!configFile.open(QIODevice::ReadOnly))
        {
            writeLog(QString("./config.xml not open,error:%1").arg(configFile.errorString()),__FILE__,__LINE__);
            return;
        }
        QDomDocument doc;
        QString errorMsg;
        int errorRow = 0;
        int errorCOl = 0;
        if (!doc.setContent(&configFile)) {
            writeLog(QString("./config.xml not read,in %1,%2 error:").arg(errorRow).arg(errorCOl).arg(errorMsg),__FILE__,__LINE__);
            configFile.close();
            return;
        }
        musicPath = doc.elementsByTagName("musicPath").at(0).firstChild().nodeValue();
        QString changeLanguage = doc.elementsByTagName("language").at(0).firstChild().nodeValue();
        audioFileFormats = doc.elementsByTagName("audioFileFormats").at(0).firstChild().nodeValue();
        qDebug()<<"music path:"<<musicPath<<" language:"<<language<<" audio formats:"<<audioFileFormats;
        configFile.close();
        this->languageChanged(changeLanguage);
    }
}

void MainWindow::languageChanged(QString lang)
{
    if(lang == "zh_CN")
    {
        this->on_actionChinese_triggered(true);
    }
    else if(lang == "en_US")
    {
        this->on_actionEnglish_triggered(true);
    }
};

void MainWindow::do_playerErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    if(error!=QMediaPlayer::Error::NoError)
    {
        writeLog(QString("playerback error:%1").arg(errorString),__FILE__,__LINE__);
        qDebug()<<"path:"<<player->source()<<" error:"<<errorString;
    }
}

void MainWindow::do_playerSeekableChanged(bool status)
{
    qDebug()<<"是否可以查找："<<status;
}

void MainWindow::do_playerBufferProgressChanged(float value)
{
    qDebug()<<"当前进度:"<<value;
}

void MainWindow::do_contextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);
    QPoint cursorPoint = QCursor::pos();
    QPoint tablePoint = ui->tableView->mapFromGlobal(cursorPoint);
    qDebug()<<"table rect:"<<ui->tableView->rect();
    qDebug()<<"widget point:"<<tablePoint;
    if(ui->tableView->rect().contains(tablePoint) && dataModel->rowCount()!=0)
    {
        QMenu* menu = new QMenu(this);
        QAction* actionCopyPath = new QAction(tr("复制路径"),this);
        QAction* actionRemove = new QAction(tr("删除"),this);
        QAction* actionClear = new QAction(tr("清空"),this);
        connect(actionClear,&QAction::triggered,this,&MainWindow::do_actionClearTriggered);
        connect(actionCopyPath,&QAction::triggered,this,&MainWindow::do_actionCopyTriggered);
        connect(actionRemove,&QAction::triggered,this,&MainWindow::do_actionRemoveTriggered);
        menu->addAction(actionCopyPath);
        menu->addAction(actionRemove);
        menu->addAction(actionClear);
        menu->exec(cursorPoint);
    }
}

void MainWindow::do_actionClearTriggered()
{
    itemSelectionModel->clearSelection();
    player->setSource(QUrl());
    labelPath->setText(QCoreApplication::translate("MainWindow","文件路径:"));
    labelDuraction->setText("");
    labelPosition->setText("");
    dataModel->clear();
}

void MainWindow::setActionLanguageStatus(bool status)
{
    if(status)
    {
        ui->actionEnglish->setChecked(true);
    }
    else
    {
        ui->actionChinese->setChecked(true);
    }
};

void MainWindow::do_actionCopyTriggered()
{
    QModelIndex index = itemSelectionModel->currentIndex();
    if(index.isValid())
    {
        QStandardItem* currentItem = dataModel->itemFromIndex(index);
        QString filePath = currentItem->data(Qt::UserRole).toString();
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(filePath);
    }
}

void MainWindow::do_actionRemoveTriggered()
{
    QModelIndex currentIndex = itemSelectionModel->currentIndex();
    if(currentIndex.isValid())
    {
        this->on_btnStop_clicked();
        int row = currentIndex.row();
        itemSelectionModel->clearCurrentIndex();
        dataModel->removeRow(row);
        if(row!=dataModel->rowCount())
        {
            QItemSelection itemSelection;
            itemSelection.select(dataModel->index(row,0),dataModel->index(row,4));
            itemSelectionModel->setCurrentIndex(dataModel->index(row,0),QItemSelectionModel::ToggleCurrent);
            itemSelectionModel->select(itemSelection,QItemSelectionModel::ToggleCurrent);
        }
    }
}

void MainWindow::do_playerSourceChanged(const QUrl& url)
{
    if(url.isLocalFile())
    {
        QString filePath = url.toLocalFile();
        QString filePathMsg = QCoreApplication::translate("MainWindow","文件路径:")+filePath;
        labelPath->setText(filePathMsg);
    }
}

void MainWindow::do_playerPlaybackRateChanged(double rate)
{
    player->setPlaybackRate(rate);
}

void MainWindow::do_playerDuractionChanged(qint64 duraction)
{
    ui->progressBar->setMaximum(duraction);
    QTime zeroTime(0,0,0);
    QTime duractionT = zeroTime.addMSecs(duraction);
    QString duractionTime = duractionT.toString("HH:mm:ss");
    labelDuraction->setText(duractionTime);
}

void MainWindow::do_playerPositionChanged(qint64 position)
{
    ui->progressBar->setValue(position);
    QTime zeroTime(0,0,0,0);
    QTime currentTime = zeroTime.addMSecs(position);
    QString positionStrTime = currentTime.toString("HH:mm:ss");
    labelPosition->setText(positionStrTime);
}

void MainWindow::do_spinValueChanged(double value)
{
    audioOutput->setVolume(value);
}

void MainWindow::do_itemSelectionRowChanged(const QModelIndex& current,const QModelIndex& previous)
{
    this->on_btnStop_clicked();
    if(current.isValid())
    {
        QStandardItem* item = dataModel->itemFromIndex(current);
        QString locationPath = item->data(Qt::UserRole).toString();
        QImage thumbnailImage = item->data().value<QImage>();
        ui->labelArtImage->setPixmap(QPixmap::fromImage(thumbnailImage));
        player->setSource(QUrl::fromLocalFile(locationPath));
    }
}

void MainWindow::do_tableViewDoubleClicked()
{

}

void MainWindow::do_stateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlaybackState::PlayingState:
        ui->btnStart->setEnabled(false);
        ui->btnPause->setEnabled(true);
        ui->btnStop->setEnabled(true);
        break;
    case QMediaPlayer::PlaybackState::PausedState:
        ui->btnStart->setEnabled(true);
        ui->btnPause->setEnabled(false);
        ui->btnStop->setEnabled(true);
        break;
    case QMediaPlayer::PlaybackState::StoppedState:
        ui->btnStart->setEnabled(true);
        ui->btnPause->setEnabled(false);
        ui->btnStop->setEnabled(false);
        break;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_progressBar_valueChanged(int value)
{
    player->setPosition(value);
}

void MainWindow::on_btnStart_clicked()
{
    if(player->playbackState()!=QMediaPlayer::PlayingState)
    {
        if(!player->isSeekable())
        {
            //isSeekable == true 才可以播放
            qDebug()<<"not seekable ,error:"<<player->errorString();
            writeLog(QString("now %1 not play,error:%2").arg(player->source().toLocalFile()).arg(player->errorString()),__FILE__,__LINE__);
            return;
        }
        player->play();
    }
}

void MainWindow::on_btnPause_clicked()
{
    if(player->playbackState()==QMediaPlayer::PlayingState)
    {
        player->pause();
    }
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    QMessageBox::Button btn = QMessageBox::question(this,tr("询问"),tr("你确定要关闭吗？"),QMessageBox::Yes|
                                                    QMessageBox::No,QMessageBox::Yes);
    if(btn==QMessageBox::Yes)
    {
        this->on_btnStop_clicked();
        int rows = dataModel->rowCount();
        QDir dir;
        if(!dir.exists("./RecentOpen"))
        {
            dir.mkdir("./RecentOpen");
        }
        QFile fileRecent("./RecentOpen/songs.txt");
        if(fileRecent.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QTextStream textStream(&fileRecent);
            textStream.setAutoDetectUnicode(true);
            textStream.setEncoding(QStringConverter::Utf32);
            textStream.setRealNumberPrecision(2);
            textStream.setIntegerBase(10);
            for(int i = 0;i<rows;i++)
            {
                QStandardItem* nowItem = dataModel->item(i,0);
                QString locationPath = nowItem->data(Qt::UserRole).toString();
                textStream<<locationPath<<"\n";
            }
            fileRecent.close();
        }

        if(configFile.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QByteArray fileData = configFile.readAll();
            configFile.close();
            QDomDocument doc;
            QString errorMsg;
            int errorRow;
            int errorColumn;
            bool ok = doc.setContent(fileData,&errorMsg,&errorRow,&errorColumn);
            if(ok)
            {
                QDomElement languageElement = doc.elementsByTagName("language").at(0).toElement();

                languageElement.firstChild().setNodeValue(language);
                QByteArray data = doc.toByteArray(4);

                if(configFile.open(QIODevice::WriteOnly))
                {
                    configFile.write(data);
                    configFile.close();
                }
                else
                {
                    writeLog(QString("./config.xml cannot read open fail,error:%1").arg(configFile.errorString()),__FILE__,__LINE__);
                }
            }
            else
            {
                writeLog(QString("./config.xml in %1,%2 space error:%3").arg(errorRow).arg(errorColumn).arg(errorMsg),__FILE__,__LINE__);
                qDebug()<<"row:"<<errorRow<<" col:"<<errorColumn<<" error:"<<errorMsg;
            }
        }
        e->accept();
    }
    else
        e->ignore();
}

void MainWindow::on_actionOpenFile_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,tr("文件选择框"),
    musicPath,QString("Audio File (%1)").arg(audioFileFormats));
    if(filePath.isEmpty())
    {
        writeLog(QString("%1 is empty").arg(filePath),__FILE__,__LINE__);
        return;
    }
    this->addRow(filePath);
}

void MainWindow::on_btnStop_clicked()
{
    if(player->playbackState()!=QMediaPlayer::StoppedState)
    {
        player->stop();
    }
}

void MainWindow::on_actionOpenDir_triggered()
{
    QString dirPath = QFileDialog::getExistingDirectory(this,tr("目录选择框"),musicPath);
    if(dirPath.isEmpty())
    {
        writeLog(QString("%1 is empty").arg(dirPath),__FILE__,__LINE__);
        return;
    }
    QDir dir(dirPath);
    QStringList fileSuffexs;
    fileSuffexs<<"*.mp3"<<"*.webm"<<"*.flac"<<"*.aac"<<"*.mpeg4audio"<<"*.quicktime";
    QFileInfoList audioFileInfos = dir.
                                   entryInfoList(
        fileSuffexs,QDir::Filter::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot,QDir::SortFlag::Time);
    foreach(auto audioFileInfo,audioFileInfos)
    {
        QString filePath = audioFileInfo.absoluteFilePath();
        this->addRow(filePath);
    }
}

void MainWindow::addRow(const QString& path)
{
    QFileInfo fileInfo(path);
    QString fileName = fileInfo.fileName();
    player->setSource(QUrl::fromLocalFile(path));
    QMediaMetaData fileMeta = player->metaData();
    QString title = fileMeta.stringValue(QMediaMetaData::AlbumTitle);
    QString author = fileMeta.stringValue(QMediaMetaData::Author);
    QString composer = fileMeta.stringValue(QMediaMetaData::Composer);
    QString date = fileMeta.stringValue(QMediaMetaData::Date);
    QVariant imageVariant = fileMeta.value(QMediaMetaData::ThumbnailImage);
    if(imageVariant.isValid())
    {
        QImage coverImage = imageVariant.value<QImage>();
        QPixmap pixmap = QPixmap::fromImage(coverImage);
        QPixmap scaledPix = pixmap.scaled(ui->labelArtImage->sizeHint());
        ui->labelArtImage->setPixmap(scaledPix);
    }
    QStandardItem* itemFileName = new QStandardItem(fileName);
    itemFileName->setData(path,Qt::UserRole);
    QStandardItem* itemTitle = new QStandardItem(title);
    QStandardItem* itemAuthor = new QStandardItem(author);
    QStandardItem* itemComposer = new QStandardItem(composer);
    QStandardItem* itemDuraction = new QStandardItem(date);
    QList<QStandardItem*> items;
    items<<itemFileName<<itemTitle<<itemAuthor<<itemComposer<<itemDuraction;
    dataModel->appendRow(items);
    QModelIndex firstIndex = dataModel->indexFromItem(itemFileName);
    QModelIndex lastIndex = dataModel->indexFromItem(itemDuraction);
    QItemSelection itemSelection;
    itemSelection.select(firstIndex,lastIndex);
    if(itemSelectionModel->hasSelection())
    {
        itemSelectionModel->clearSelection();
    }
    itemSelectionModel->select(itemSelection,QItemSelectionModel::ToggleCurrent);
};

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    if(e->mimeData()->hasUrls())
    {
        QList<QUrl> urls = e->mimeData()->urls();
        for(auto url:urls)
        {
            if(url.isLocalFile())
            {
                QFileInfo fileInfo(url.toLocalFile());
                QString suffix = fileInfo.suffix().toLower();
                if(suffix=="mp3" || suffix=="aac" || suffix=="flac")
                {
                    paths.append(url.toLocalFile());
                }
            }
        }
        if(paths.isEmpty())
        {
            e->ignore();
        }
        else
            e->acceptProposedAction();
    }
};

void MainWindow::dropEvent(QDropEvent* e)
{
    for(auto path:paths)
    {
        this->addRow(path);
    }
    e->accept();
};

void MainWindow::initRecentMenu()
{
    QDir dir("./RecentOpen");
    if(!dir.exists())
    {
        QDir::current().mkdir(dir.absolutePath());
    }
    if(!dir.exists("songs.txt"))
    {
        writeLog(QString("%1 is not exists").arg(dir.absoluteFilePath("songs.txt")),__FILE__,__LINE__);
        return;
    }
    else
    {
        qDebug()<<"songs.txt 存在";
        QString absoultFilePath = dir.absoluteFilePath("songs.txt");
        QFile file(absoultFilePath);
        if(file.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QTextStream textStream(&file);
            textStream.setAutoDetectUnicode(true);
            textStream.setEncoding(QStringConverter::Utf32);
            textStream.setRealNumberPrecision(2);
            textStream.setIntegerBase(10);
            QString fileContext = textStream.readAll();
            file.close();
            QStringList filePaths = fileContext.split("\n");
            for(auto filePath:filePaths)
            {
                if(filePath.isEmpty())
                {
                    continue;
                }
                else
                {
                    QAction* filePathAction = new QAction(filePath,this);
                    menuRecent->addAction(filePathAction);
                }
            }
        }
        else
        {
            writeLog(QString("%1 open fail,error:%2").arg(absoultFilePath).arg(file.errorString()),__FILE__,__LINE__);
        }
    }
}

void MainWindow::do_menuTriggered(QAction* action)
{
    if(action!=nullptr)
    {
        QString actionText = action->text();
        this->addRow(actionText);
    }
}

extern QTranslator translator;

void MainWindow::on_lineEditSearch_editingFinished()
{
    QString text = ui->lineEditSearch->text();
    QList<QStandardItem*> items = dataModel->findItems(text,Qt::MatchExactly);
    if(items.isEmpty())
    {
        QMessageBox::information(this,tr("提示"),tr("找不到该音乐"));

        return;
    }
    else
    {
        QStandardItem* item = items.at(0);
        int itemRow = item->row();
        QItemSelection itemSelection;
        itemSelection.select(dataModel->index(itemRow,0),dataModel->index(itemRow,4));
        itemSelectionModel->setCurrentIndex(dataModel->index(itemRow,0),QItemSelectionModel::ToggleCurrent);
        itemSelectionModel->select(itemSelection,QItemSelectionModel::ToggleCurrent);
        ui->tableView->scrollTo(dataModel->indexFromItem(item));
    }
}

void MainWindow::on_actionEnglish_triggered(bool checked)
{
    if(checked)
    {
        qDebug()<<"英文";
        if(translator.load(":/i18n/AudioPlayerback_en.qm"))
        {
            ui->retranslateUi(this);
            QString text = translator.translate("MainWindow","最近打开的文件");
            menuRecent->setTitle(text);
            for(int i = 0;i<5;i++)
            {
                qDebug()<<"en_US cStr:"<<headers.at(i).toStdString().c_str();
                QString translaorText = translator.translate("MainWindow",headers.at(i).toStdString().c_str());
                qDebug()<<"en_US translator text:"<<translaorText;
                dataModel->horizontalHeaderItem(i)->setText(translator.translate("MainWindow",headers.at(i).toStdString().c_str()));
            }
            volumeSpix->setSuffix(translator.translate("MainWindow","音量"));
            rateSpin->setSuffix(translator.translate("MainWindow","倍速"));
            language = translator.language();
        }
        else
        {
            qDebug()<<":/i18n/AudioPlayerback_en.qm加载失败";
            writeLog(":/i18n/AudioPlayerback_en.qm加载失败",__FILE__,__LINE__);
        }
    }
}

void MainWindow::on_actionChinese_triggered(bool checked)
{
    if(checked)
    {
        qDebug()<<"中文";
        if(translator.load(":/i18n/AudioPlayerback_zh_CN.qm"))
        {
            ui->retranslateUi(this);
            QString text = translator.translate("MainWindow","最近打开的文件");
            qDebug()<<"translator："<<text;
            menuRecent->setTitle(text);
            for(int i = 0;i<5;i++)
            {
                qDebug()<<"zh_CN cStr:"<<headers.at(i).toStdString().c_str();
                QString translaorText = translator.translate("MainWindow",headers.at(i).toStdString().c_str());
                qDebug()<<"zh_CN translator text:"<<translaorText;
                dataModel->horizontalHeaderItem(i)->setText(translator.translate("MainWindow",headers.at(i).toStdString().c_str()));
            }
            volumeSpix->setSuffix(translator.translate("MainWindow","音量"));
            rateSpin->setSuffix(translator.translate("MainWindow","倍速"));
            language = translator.language();
        }
        else
        {
            writeLog(":/i18n/AudioPlayerback_zh_CN.qm加载失败",__FILE__,__LINE__);
        }
    }
}

