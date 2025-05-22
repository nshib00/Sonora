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

AudioPlayer::AudioPlayer(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Sonora");
    setAcceptDrops(true);

    dbManager = new DBManager(this);
    dbManager->init("audioplayer.db");

    setupUi();
    setupConnections();
}

QIcon loadColoredIcon(const QString &resourcePath, const QColor &color)
{
    QSvgRenderer renderer(resourcePath);

    QSize svgSize = renderer.defaultSize();
    if (!svgSize.isValid()) svgSize = QSize(64, 64);

    QPixmap pixmap(svgSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return QIcon(pixmap);
}

void AudioPlayer::setupUi()
{
    player = new QMediaPlayer(this);
    trackManager = new TrackManager(this);

    openButton = new QPushButton;
    openButton->setIcon(loadColoredIcon(":/images/icons/file.svg", Qt::white));

    openFolderButton = new QPushButton;
    openFolderButton->setIcon(loadColoredIcon(":/images/icons/folder.svg", Qt::white));

    playPauseButton = new QPushButton;
    playPauseButton->setIcon(loadColoredIcon(":/images/icons/play.svg", Qt::white));

    stopButton = new QPushButton;
    stopButton->setIcon(loadColoredIcon(":/images/icons/stop.svg", Qt::white));

    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 0);

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    player->setVolume(50);

    leftTimeLabel = new QLabel("00:00");
    rightTimeLabel = new QLabel("00:00");

    trackTitleLabel = new QLabel("Трек не выбран");
    trackTitleLabel->setAlignment(Qt::AlignCenter);

    albumTitleLabel = new QLabel("");
    albumTitleLabel->setAlignment(Qt::AlignCenter);

    coverArtLabel = new QLabel;
    coverArtLabel->setMinimumSize(300, 300);
    coverArtLabel->setMaximumSize(600, 600);
    coverArtLabel->setAlignment(Qt::AlignCenter);
    coverArtLabel->setPixmap(QPixmap());

    trackListWidget = new QListWidget;
    trackListWidget->setMinimumWidth(200);
    trackListWidget->setMaximumWidth(400);

    prevButton = new QPushButton;
    prevButton->setIcon(loadColoredIcon(":/images/icons/skip-back.svg", Qt::white));

    nextButton = new QPushButton;
    nextButton->setIcon(loadColoredIcon(":/images/icons/skip-forward.svg", Qt::white));

    repeatButton = new QPushButton;
    repeatButton->setIcon(loadColoredIcon(":/images/icons/refresh-ccw.svg", Qt::white));

    shuffleButton = new QPushButton;
    shuffleButton->setIcon(loadColoredIcon(":/images/icons/shuffle.svg", Qt::white));
    shuffleButton->setText(" выкл");

    auto *openAudioLayout = new QHBoxLayout;
    openAudioLayout->addWidget(openButton);
    openAudioLayout->addWidget(openFolderButton);
    openAudioLayout->addStretch();

    auto *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(repeatButton);
    buttonsLayout->addWidget(shuffleButton);

    auto *volumeLayout = new QHBoxLayout;
    volumeLayout->addWidget(new QLabel("Громкость"));
    volumeLayout->addWidget(volumeSlider);

    auto *timeLayout = new QHBoxLayout;
    timeLayout->addWidget(leftTimeLabel);
    timeLayout->addWidget(progressSlider);
    timeLayout->addWidget(rightTimeLabel);

    auto *navLayout = new QHBoxLayout;
    navLayout->addWidget(prevButton);
    navLayout->addWidget(playPauseButton);
    navLayout->addWidget(stopButton);
    navLayout->addWidget(nextButton);

    auto *audioPlayerLayout = new QVBoxLayout;
    audioPlayerLayout->addLayout(openAudioLayout);
    audioPlayerLayout->addWidget(coverArtLabel, 1, Qt::AlignCenter);
    audioPlayerLayout->addWidget(trackTitleLabel);
    audioPlayerLayout->addWidget(albumTitleLabel);
    audioPlayerLayout->addLayout(buttonsLayout);
    audioPlayerLayout->addLayout(timeLayout);
    audioPlayerLayout->addLayout(navLayout);
    audioPlayerLayout->addLayout(volumeLayout);
    audioPlayerLayout->setMargin(15);

    playlistWidget = new PlaylistWidget(dbManager, this);
    playlistWidget->setMinimumWidth(200);
    playlistWidget->setMaximumWidth(250);

    auto *mainLayout = new QHBoxLayout(this);

    mainLayout->addWidget(playlistWidget);
    mainLayout->addLayout(audioPlayerLayout);
    mainLayout->addWidget(trackListWidget);

    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    applyStyles();
}


void AudioPlayer::setupConnections()
{
    connect(openButton, &QPushButton::clicked, this, &AudioPlayer::openFile);
    connect(openFolderButton, &QPushButton::clicked, this, &AudioPlayer::openFolder);
    connect(stopButton, &QPushButton::clicked, player, &QMediaPlayer::stop);
    connect(volumeSlider, &QSlider::valueChanged, player, &QMediaPlayer::setVolume);
    connect(player, &QMediaPlayer::positionChanged, this, &AudioPlayer::updatePosition);
    connect(player, &QMediaPlayer::durationChanged, this, &AudioPlayer::updateDuration);
    connect(progressSlider, &QSlider::sliderMoved, this, &AudioPlayer::setPosition);
    connect(player, &QMediaPlayer::metaDataAvailableChanged, this, &AudioPlayer::updateMetaData);

    connect(playPauseButton, &QPushButton::clicked, this, [this]() {
        if (player->state() == QMediaPlayer::PlayingState) {
            player->pause();
        } else {
            player->play();
        }
    });

    connect(player, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::PlayingState) {
            playPauseButton->setIcon(loadColoredIcon(":/images/icons/pause.svg", Qt::white));
        } else {
            playPauseButton->setIcon(loadColoredIcon(":/images/icons/play.svg", Qt::white));
        }
    });

    // Навигация треков напрямую
    connect(prevButton, &QPushButton::clicked, trackManager, &TrackManager::playPrevious);
    connect(nextButton, &QPushButton::clicked, trackManager, &TrackManager::playNext);

    // Повтор — toggle + обновляем надпись кнопки
    connect(repeatButton, &QPushButton::clicked, this, [this]() {
        trackManager->toggleRepeat();
        switch (trackManager->repeatMode()) {
        case TrackManager::NoRepeat: repeatButton->setText(" выкл"); break;
        case TrackManager::RepeatAll: repeatButton->setText(" все"); break;
        case TrackManager::RepeatOne: repeatButton->setText(" трек"); break;
        }
    });

    // Перемешивание
    connect(shuffleButton, &QPushButton::clicked, this, [this]() {
        trackManager->toggleShuffle();
        shuffleButton->setText(trackManager->shuffleEnabled() ? "вкл" : "выкл");
    });

    // Обновление плейлиста
    connect(trackManager, &TrackManager::playlistUpdated, this, [this](const QStringList &files) {
        trackListWidget->clear();
        for (const QString &filePath : files) {
            QFileInfo fi(filePath);
            trackListWidget->addItem(fi.fileName());
        }
    });

    // Обновление активного трека
    connect(trackManager, &TrackManager::trackChanged, this, [this](const QString &filePath, int index) {
        player->setMedia(QUrl::fromLocalFile(filePath));
        player->play();
        trackListWidget->setCurrentRow(index);
    });

    connect(trackListWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        int index = trackListWidget->row(item);
        trackManager->playTrack(index);
    });

    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            trackManager->playNext();
        }
    });

    connect(playlistWidget, &PlaylistWidget::playlistSelected, this, [this](int playlistId) {
        QStringList tracks = dbManager->loadPlaylistTracks(playlistId);
        if (!tracks.isEmpty()) {
            trackManager->setPlaylist(tracks);
            trackManager->playTrack(0);
        }
    });

    connect(playlistWidget, &PlaylistWidget::trackAddedToPlaylist, this, [this](int playlistId) {
        QStringList tracks = dbManager->loadPlaylistTracks(playlistId);
        if (!tracks.isEmpty()) {
            trackManager->setPlaylist(tracks);
            trackListWidget->clear();
            for (const QString &filePath : tracks) {
                QFileInfo fi(filePath);
                trackListWidget->addItem(fi.fileName());
            }
        }
    });
}

void AudioPlayer::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть аудио", "", "Audio Files (*.mp3 *.wav)");
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
    if (!progressSlider->isSliderDown()) {
        progressSlider->setValue(static_cast<int>(position));
    }
    updateTimeLabels();
}

void AudioPlayer::updateDuration(qint64 dur)
{
    duration = dur;
    progressSlider->setRange(0, static_cast<int>(duration));
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
    leftTimeLabel->setText(currentTime.toString(format));
    rightTimeLabel->setText(totalTime.toString(format));
}

void AudioPlayer::updateMetaData()
{
    qDebug() << player->availableMetaData();

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

    trackTitleLabel->setText(artist + " - " + title);
    albumTitleLabel->setText(albumTitle);

    QVariant cover = player->metaData(QMediaMetaData::ThumbnailImage);
    QPixmap pixmap;
    if (cover.isValid()) {
        pixmap = cover.value<QPixmap>();
    } else {
        pixmap.load(":/images/placeholder.png");
    }

    QPixmap scaled = pixmap.scaled(coverArtLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    coverArtLabel->setPixmap(scaled);
}


void AudioPlayer::applyStyles()
{
    QFile file(":/css/audioplayer.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        this->setStyleSheet(styleSheet);
    } else {
        qDebug() << "Ошибка при открытии файла стилей!";
    }
}
