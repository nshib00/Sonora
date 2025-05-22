#include "audioplayer.h"
#include "trackmanager.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QSlider>
#include <QLabel>
#include <QTime>
#include <QMediaMetaData>
#include <QPixmap>
#include <QStyle>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QMenu>
#include <QListWidget>

#include "audioplayerui.h"
#include "dbmanager.h"
#include "playlistwidget.h"

AudioPlayer::AudioPlayer(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
    setWindowTitle("Sonora");

    dbManager = new DBManager(this);
    dbManager->init("audioplayer.db");

    player = new QMediaPlayer(this);
    trackManager = new TrackManager(this);

    ui = new AudioPlayerUI(dbManager, this);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(ui);
    setLayout(layout);

    player->setVolume(ui->volumeSlider->value());

    setupConnections();
}

void AudioPlayer::setupConnections()
{
    connect(ui->openButton, &QPushButton::clicked, this, &AudioPlayer::openFile);
    connect(ui->openFolderButton, &QPushButton::clicked, this, &AudioPlayer::openFolder);
    connect(ui->stopButton, &QPushButton::clicked, player, &QMediaPlayer::stop);
    connect(ui->volumeSlider, &QSlider::valueChanged, player, &QMediaPlayer::setVolume);
    connect(player, &QMediaPlayer::positionChanged, this, &AudioPlayer::updatePosition);
    connect(player, &QMediaPlayer::durationChanged, this, &AudioPlayer::updateDuration);
    connect(ui->progressSlider, &QSlider::sliderMoved, this, &AudioPlayer::setPosition);
    connect(player, &QMediaPlayer::metaDataAvailableChanged, this, &AudioPlayer::updateMetaData);

    connect(ui->trackListWidget, &QListWidget::customContextMenuRequested, this, &AudioPlayer::showTrackContextMenu);


    connect(ui->playPauseButton, &QPushButton::clicked, this, [this]() {
        if (player->state() == QMediaPlayer::PlayingState) {
            player->pause();
        } else {
            player->play();
        }
    });

    connect(player, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::PlayingState) {
            ui->playPauseButton->setIcon(ui->loadColoredIcon(":/images/icons/pause.svg", Qt::white));
        } else {
            ui->playPauseButton->setIcon(ui->loadColoredIcon(":/images/icons/play.svg", Qt::white));
        }
    });

    // Навигация треков напрямую
    connect(ui->prevButton, &QPushButton::clicked, trackManager, &TrackManager::playPrevious);
    connect(ui->nextButton, &QPushButton::clicked, trackManager, &TrackManager::playNext);

    // Повтор — toggle + обновляем надпись кнопки
    connect(ui->repeatButton, &QPushButton::clicked, this, [this]() {
        trackManager->toggleRepeat();
        switch (trackManager->repeatMode()) {
        case TrackManager::NoRepeat:
            ui->repeatButton->setText(" выкл"); break;
        case TrackManager::RepeatAll:
            ui->repeatButton->setText(" все"); break;
        case TrackManager::RepeatOne:
            ui->repeatButton->setText(" трек"); break;
        }
    });

    // Перемешивание
    connect(ui->shuffleButton, &QPushButton::clicked, this, [this]() {
        trackManager->toggleShuffle();
        ui->shuffleButton->setText(trackManager->shuffleEnabled() ? "вкл" : "выкл");
    });

    // Обновление плейлиста
    connect(trackManager, &TrackManager::playlistUpdated, this, [this](const QStringList &files) {
        ui->trackListWidget->clear();
        for (const QString &filePath : files) {
            QFileInfo fi(filePath);
            ui->trackListWidget->addItem(fi.fileName());
        }
    });

    // Обновление активного трека
    connect(trackManager, &TrackManager::trackChanged, this, [this](const QString &filePath, int index) {
        player->setMedia(QUrl::fromLocalFile(filePath));
        player->play();
        ui->trackListWidget->setCurrentRow(index);
    });

    connect(ui->trackListWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        int index = ui->trackListWidget->row(item);
        trackManager->playTrack(index);
    });

    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            trackManager->playNext();
        }
    });

    connect(ui->playlistWidget, &PlaylistWidget::playlistSelected, this, [this](int playlistId) {
        QStringList tracks = dbManager->loadPlaylistTracks(playlistId);
        if (!tracks.isEmpty()) {
            trackManager->setPlaylist(tracks);
            trackManager->playTrack(0);
        }
    });

    connect(ui->playlistWidget, &PlaylistWidget::trackAddedToPlaylist, this, [this](int playlistId) {
        QStringList tracks = dbManager->loadPlaylistTracks(playlistId);
        if (!tracks.isEmpty()) {
            trackManager->setPlaylist(tracks);
            ui->trackListWidget->clear();
            for (const QString &filePath : tracks) {
                QFileInfo fi(filePath);
                ui->trackListWidget->addItem(fi.fileName());
            }
        }
    });
}

void AudioPlayer::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть аудио", "", "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a)");
    if (!fileName.isEmpty()) {
        trackManager->setPlaylist({fileName});
        trackManager->playTrack(0);
    }
}

void AudioPlayer::openFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Выбрать папку с аудиофайлами");
    if (!folderPath.isEmpty()) {
        QDir dir(folderPath);
        QStringList filters = {"*.mp3", "*.wav", "*.flac", "*.ogg", "*.m4a"};
        QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

        QStringList files;
        for (const QFileInfo &fileInfo : fileList) {
            files << fileInfo.absoluteFilePath();
        }

        if (!files.isEmpty()) {
            trackManager->setPlaylist(files);
            trackManager->playTrack(0);
        }
    }
}

void AudioPlayer::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void AudioPlayer::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;

    QStringList allowedExtensions = {"mp3", "wav", "flac", "ogg", "m4a"};
    QStringList newFiles;

    for (const QUrl &url : urls) {
        QString path = url.toLocalFile();
        if (path.isEmpty()) continue;

        QFileInfo fileInfo(path);

        if (fileInfo.isDir()) {
            QDir dir(path);
            QStringList filters;
            for (const QString &ext : allowedExtensions) {
                filters << "*." + ext;
            }
            QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
            for (const QFileInfo &audioFile : fileList) {
                newFiles << audioFile.absoluteFilePath();
            }
        } else {
            QString ext = fileInfo.suffix().toLower();
            if (allowedExtensions.contains(ext)) {
                newFiles << fileInfo.absoluteFilePath();
            }
        }
    }

    if (!newFiles.isEmpty()) {
        trackManager->setPlaylist(newFiles);
        trackManager->playTrack(0);
    }
}

void AudioPlayer::updatePosition(qint64 position)
{
    if (!ui->progressSlider->isSliderDown()) {
        ui->progressSlider->setValue(static_cast<int>(position));
    }
    updateTimeLabels();
}

void AudioPlayer::updateDuration(qint64 dur)
{
    duration = dur;
    ui->progressSlider->setRange(0, static_cast<int>(duration));
    updateTimeLabels();
}

void AudioPlayer::setPosition(int position)
{
    player->setPosition(position);
}

void AudioPlayer::updateTimeLabels()
{
    QTime currentTime((player->position() / 3600000) % 60,
                      (player->position() / 60000) % 60,
                      (player->position() / 1000) % 60);
    QTime totalTime((duration / 3600000) % 60,
                    (duration / 60000) % 60,
                    (duration / 1000) % 60);

    QString format = (duration > 3600000) ? "hh:mm:ss" : "mm:ss";
    ui->leftTimeLabel->setText(currentTime.toString(format));
    ui->rightTimeLabel->setText(totalTime.toString(format));
}

void AudioPlayer::updateMetaData()
{
      QString artist = player->metaData(QMediaMetaData::Author).toString();
    QString title = player->metaData(QMediaMetaData::Title).toString();
    QString albumTitle = player->metaData(QMediaMetaData::AlbumTitle).toString();
    QString trackYear = player->metaData(QMediaMetaData::Year).toString();

    QString fallbackArtist = "Неизвестен";
    QString fallbackTitle = "Без названия";

    if (title.isEmpty() || artist.isEmpty()) {
        // Пытаемся извлечь из имени файла
        QString filePath = player->currentMedia().canonicalUrl().toLocalFile();
        QFileInfo fileInfo(filePath);
        QString baseName = fileInfo.completeBaseName(); // без расширения

        // Попытка разбора: "Artist - Title"
        QStringList parts = baseName.split(" - ");
        if (parts.size() == 2) {
            if (artist.isEmpty()) artist = parts[0].trimmed();
            if (title.isEmpty()) title = parts[1].trimmed();
        } else {
            // Если нет разделителя, всё идёт в title
            if (title.isEmpty()) title = baseName;
        }
    }

    if (artist.isEmpty())
        artist = fallbackArtist;
    if (title.isEmpty())
        title = fallbackTitle;
    if (albumTitle.isEmpty())
        albumTitle = "Неизвестный альбом";
    if (!trackYear.isEmpty() && trackYear != 0)
        albumTitle += " (" + trackYear + ")";

    ui->trackTitleLabel->setText(artist + " - " + title);
    ui->albumTitleLabel->setText(albumTitle);

    QVariant cover = player->metaData(QMediaMetaData::ThumbnailImage);
    QPixmap pixmap;
    if (cover.isValid()) {
        pixmap = cover.value<QPixmap>();
    } else {
        pixmap.load(":/images/placeholder.png");
    }

    QPixmap scaled = pixmap.scaled(ui->coverArtLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->coverArtLabel->setPixmap(scaled);
}


void AudioPlayer::displayPlaylistTracks(const QStringList &tracks, int playlistId)
{
    ui->trackListWidget->clear();

    for (const QString &filePath : tracks) {
        QFileInfo fi(filePath);
        QListWidgetItem *item = new QListWidgetItem(fi.fileName());
        item->setData(Qt::UserRole, filePath);           // путь к файлу
        item->setData(Qt::UserRole + 1, playlistId);     // ID плейлиста
        ui->trackListWidget->addItem(item);
    }

    // Устанавливаем текущий плейлист
    trackManager->setPlaylist(tracks);
    trackManager->playTrack(0);
}

void AudioPlayer::showTrackContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = ui->trackListWidget->itemAt(pos);
    if (!item) return;

    QString filePath = item->data(Qt::UserRole).toString();
    int playlistId = item->data(Qt::UserRole + 1).toInt();

    // Контекстное меню
    QMenu menu;
    QAction *removeAction = menu.addAction("Удалить из плейлиста");

    QAction *selectedAction = menu.exec(ui->trackListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == removeAction) {
        if (playlistId > 0) {
            dbManager->removeTrackFromPlaylist(playlistId, filePath);

            // Обновляем треклист
            QStringList updatedTracks = dbManager->loadPlaylistTracks(playlistId);
            displayPlaylistTracks(updatedTracks, playlistId);
        }
    }
}

