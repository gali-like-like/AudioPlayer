#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setAcceptDrops(true);
    ui->tableView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->tableView->setAcceptDrops(false);
    rateSpin = new QDoubleSpinBox(this);
    rateSpin->setRange(0,30);
    rateSpin->setSingleStep(0.25);
    rateSpin->setSuffix("倍速");
    labelPath = new QLabel("文件路径:",this);
    labelDuraction = new QLabel(this);
    labelPosition = new QLabel(this);
    ui->statusbar->addWidget(labelPath,4);
    ui->statusbar->addWidget(labelPosition,1);
    ui->statusbar->addWidget(labelDuraction,1);
    volumeSpix = new QDoubleSpinBox(this);
    volumeSpix->setRange(0.00,1.00);
    volumeSpix->setSingleStep(0.01);
    volumeSpix->setSuffix("音量");
    connect(volumeSpix,&QDoubleSpinBox::valueChanged,this,&MainWindow::do_spinValueChanged);
    widgetAction = new QWidgetAction(this);
    QWidgetAction* widgetRateAction = new QWidgetAction(this);
    widgetRateAction->setDefaultWidget(rateSpin);
    widgetAction->setDefaultWidget(volumeSpix);
    ui->menubar->addAction(ui->actionOpenFile);
    ui->menubar->addAction(ui->actionOpenDir);
    menuRecent = new QMenu("最近打开的文件",this);
    connect(menuRecent,&QMenu::triggered,this,&MainWindow::do_menuTriggered);
    ui->menubar->addMenu(menuRecent);
    ui->toolBar->addAction(widgetAction);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(widgetRateAction);
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
    QStringList headers;
    headers<<"文件名"<<"标题"<<"作者"<<"作曲人"<<"时间";
    for(int i = 0;i<5;i++)
    {
        QStandardItem* item = new QStandardItem();
        item->setText(headers.at(i));
        item->setTextAlignment(Qt::AlignCenter);
        item->setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
        item->setForeground(QBrush(QGradient(QGradient::Preset::BigMango)));
        dataModel->setHorizontalHeaderItem(i,item);
    }
    itemSelectionModel = new QItemSelectionModel(dataModel,this);
    ui->tableView->setModel(dataModel);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setSelectionModel(itemSelectionModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setResizeContentsPrecision(0);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(player,&QMediaPlayer::playbackStateChanged,this,&MainWindow::do_stateChanged);
    connect(itemSelectionModel,&QItemSelectionModel::currentRowChanged,this,&MainWindow::do_itemSelectionRowChanged);
    connect(player,&QMediaPlayer::positionChanged,this,&MainWindow::do_playerPositionChanged);
    connect(player,&QMediaPlayer::durationChanged,this,&MainWindow::do_playerDuractionChanged);
    connect(player,&QMediaPlayer::sourceChanged,this,&MainWindow::do_playerSourceChanged);
    connect(player,&QMediaPlayer::bufferProgressChanged,this,&MainWindow::do_playerBufferProgressChanged);
    connect(ui->tableView,&QTableView::customContextMenuRequested,this,&MainWindow::do_contextMenuRequested);
    connect(player,&QMediaPlayer::seekableChanged,this,&MainWindow::do_playerSeekableChanged);
    initRecentMenu();
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
    QPoint tablePoint = this->mapFromGlobal(cursorPoint);
    qDebug()<<"table rect:"<<ui->tableView->rect();
    qDebug()<<"widget point:"<<tablePoint;
    if(ui->tableView->rect().contains(tablePoint) && dataModel->rowCount()!=0)
    {
        QMenu* menu = new QMenu(this);
        QAction* actionCopyPath = new QAction("复制路径",this);
        QAction* actionRemove = new QAction("删除",this);
        QAction* actionClear = new QAction("清空",this);
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
    labelPath->setText("文件路径:");
    labelDuraction->setText("");
    labelPosition->setText("");
    dataModel->clear();
}

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
            itemSelectionModel->select(itemSelection,QItemSelectionModel::ToggleCurrent);
            itemSelectionModel->setCurrentIndex(dataModel->index(row,0),QItemSelectionModel::ToggleCurrent);
//            this->do_itemSelectionRowChanged(dataModel->index(row,0),currentIndex);
        }
    }
}

void MainWindow::do_playerSourceChanged(const QUrl& url)
{
    if(url.isLocalFile())
    {
        QString filePath = url.toLocalFile();
        labelPath->setText(QString(tr("文件路径:%1").arg(filePath)));
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
            qDebug()<<"not seekable ,error:"<<player->errorString();
            return;
        }
        bool ok = mediaFormat.isSupported(QMediaFormat::Decode);
        qDebug()<<"support ogg decord?"<<ok;
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
    QMessageBox::Button btn = QMessageBox::question(this,tr("询问"),"你确定要关闭吗？",QMessageBox::Yes|
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
        e->accept();
    }
    else
        e->ignore();
}

void MainWindow::on_actionOpenFile_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,tr("文件选择框"),
    "E:\\QQMusicDown","Audio File (*.mp3 *.flac *.webm *.aac *.quicktime *.mpeg4audio)");
    if(filePath.isEmpty())
    {
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
    QString dirPath = QFileDialog::getExistingDirectory(this,tr("目录选择框"),"E:\\QQMusicDown");
    if(dirPath.isEmpty())
    {
        return;
    }
    QDir dir(dirPath);
    QStringList fileSuffexs;
    fileSuffexs<<"*.mp3"<<"*.webm"<<"*.flac"<<"*.aac"<<"*.mpeg4audio"<<"*.quicktime";
    QFileInfoList audioFileInfos = dir.
                                   entryInfoList(
        fileSuffexs,QDir::Filter::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot,QDir::SortFlag::Time);
    qDebug()<<"files："<<audioFileInfos;
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
    player->setSource(QUrl::fromLocalFile(fileName));
    QMediaMetaData fileMeta = player->metaData();
    QString title = fileMeta.stringValue(QMediaMetaData::AlbumTitle);
    QString author = fileMeta.stringValue(QMediaMetaData::Author);
    QString composer = fileMeta.stringValue(QMediaMetaData::Composer);
    QString date = fileMeta.stringValue(QMediaMetaData::Date);
    QVariant imageVariant = fileMeta.value(QMediaMetaData::ThumbnailImage);
    qDebug()<<"image variant is valid?"<<imageVariant.isValid();
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
    itemSelectionModel->select(itemSelection,QItemSelectionModel::Current);
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
            qDebug()<<"file context:"<<fileContext;
            file.close();
            QStringList filePaths = fileContext.split("\n");
            for(auto filePath:filePaths)
            {
                QAction* filePathAction = new QAction(filePath,this);
                menuRecent->addAction(filePathAction);
            }
        }
        else
        {
            qDebug()<<"songs.txt 打开失败";
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

