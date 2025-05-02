#include "audioplayer.h"

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
#include <QRandomGenerator>

#include <QDebug>


AudioPlayer::AudioPlayer(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupConnections();
}

void AudioPlayer::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть аудио", "", "Audio Files (*.mp3 *.wav)");
    if (!fileName.isEmpty()) {
        player->setMedia(QUrl::fromLocalFile(fileName));
        player->play();
    }
}


void AudioPlayer::setupUi()
{
    setWindowTitle("Sonora");
    setAcceptDrops(true);

    player = new QMediaPlayer(this);

    openButton = new QPushButton;
    openButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));

    openFolderButton = new QPushButton("Открыть папку");

    playButton = new QPushButton;
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    pauseButton = new QPushButton;
    pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    stopButton = new QPushButton;
    stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 0);

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    player->setVolume(50);

    timeLabel = new QLabel("00:00 / 00:00");

    trackTitleLabel = new QLabel("Трек не выбран");
    trackTitleLabel->setAlignment(Qt::AlignCenter);

    albumTitleLabel = new QLabel("");
    albumTitleLabel->setAlignment(Qt::AlignCenter);

    coverArtLabel = new QLabel;
    coverArtLabel->setFixedSize(250, 250);
    coverArtLabel->setAlignment(Qt::AlignCenter);
    coverArtLabel->setPixmap(QPixmap());

    trackListWidget = new QListWidget;
    trackListWidget->setMinimumWidth(200);
    trackListWidget->setMaximumWidth(400);

    prevButton = new QPushButton;
    prevButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    nextButton = new QPushButton;
    nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    repeatButton = new QPushButton;
    repeatButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

    shuffleButton = new QPushButton;
    shuffleButton->setText("🔀 выкл");

    auto *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(openButton);
    buttonsLayout->addWidget(playButton);
    buttonsLayout->addWidget(pauseButton);
    buttonsLayout->addWidget(stopButton);
    buttonsLayout->addWidget(repeatButton);
    buttonsLayout->addWidget(shuffleButton);

    auto *folderLayout = new QHBoxLayout;
    folderLayout->addWidget(openFolderButton);

    auto *volumeLayout = new QHBoxLayout;
    volumeLayout->addWidget(new QLabel("Громкость"));
    volumeLayout->addWidget(volumeSlider);

    auto *navLayout = new QHBoxLayout;
    navLayout->addWidget(prevButton);
    navLayout->addWidget(progressSlider);
    navLayout->addWidget(nextButton);

    auto *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(coverArtLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(trackTitleLabel);
    leftLayout->addWidget(albumTitleLabel);
    leftLayout->addLayout(buttonsLayout);
    leftLayout->addLayout(folderLayout);
    leftLayout->addLayout(navLayout);
    leftLayout->addWidget(timeLabel);
    leftLayout->addLayout(volumeLayout);

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(trackListWidget);

    applyStyles();
}


void AudioPlayer::setupConnections()
{
    connect(openButton, &QPushButton::clicked, this, &AudioPlayer::openFile);
    connect(openFolderButton, &QPushButton::clicked, this, &AudioPlayer::openFolder);
    connect(playButton, &QPushButton::clicked, player, &QMediaPlayer::play);
    connect(pauseButton, &QPushButton::clicked, player, &QMediaPlayer::pause);
    connect(stopButton, &QPushButton::clicked, player, &QMediaPlayer::stop);
    connect(volumeSlider, &QSlider::valueChanged, player, &QMediaPlayer::setVolume);
    connect(player, &QMediaPlayer::positionChanged, this, &AudioPlayer::updatePosition);
    connect(player, &QMediaPlayer::durationChanged, this, &AudioPlayer::updateDuration);
    connect(progressSlider, &QSlider::sliderMoved, this, &AudioPlayer::setPosition);
    connect(player, &QMediaPlayer::metaDataAvailableChanged, this, &AudioPlayer::updateMetaData);
    connect(trackListWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
            int index = trackListWidget->row(item);
            playTrack(index);
    });
    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            playNext();
        }
    });
    connect(prevButton, &QPushButton::clicked, this, &AudioPlayer::playPrevious);
    connect(nextButton, &QPushButton::clicked, this, &AudioPlayer::playNext);
    connect(repeatButton, &QPushButton::clicked, this, &AudioPlayer::toggleRepeatMode);
    connect(shuffleButton, &QPushButton::clicked, this, &AudioPlayer::toggleShuffleMode);
}


void AudioPlayer::openFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Выбрать папку с аудиофайлами");
    if (!folderPath.isEmpty()) {
        QDir dir(folderPath);
        QStringList filters = {"*.mp3", "*.wav", "*.flac", "*.ogg", "*.m4a"};
        QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

        playlistFiles.clear();
        trackListWidget->clear();

        for (const QFileInfo &fileInfo : fileList) {
            playlistFiles << fileInfo.absoluteFilePath();
            trackListWidget->addItem(fileInfo.fileName());
        }

        if (!playlistFiles.isEmpty()) {
            playTrack(0);
        }
    }
}

void AudioPlayer::updatePosition(qint64 position)
{
    if (!progressSlider->isSliderDown()) {
        progressSlider->setValue(static_cast<int>(position));
    }
    updateTimeLabel();
}

void AudioPlayer::updateDuration(qint64 dur)
{
    duration = dur;
    progressSlider->setRange(0, static_cast<int>(duration));
    updateTimeLabel();
}

void AudioPlayer::setPosition(int position)
{
    player->setPosition(position);
}

void AudioPlayer::updateTimeLabel()
{
    QTime currentTime((player->position() / 3600000) % 60,
                      (player->position() / 60000) % 60,
                      (player->position() / 1000) % 60);
    QTime totalTime((duration / 3600000) % 60,
                    (duration / 60000) % 60,
                    (duration / 1000) % 60);

    QString format = (duration > 3600000) ? "hh:mm:ss" : "mm:ss";
    timeLabel->setText(currentTime.toString(format) + " / " + totalTime.toString(format));
}

void AudioPlayer::updateMetaData()
{
    QString artist = player->metaData(QMediaMetaData::Author).toString();
    QString title = player->metaData(QMediaMetaData::Title).toString();
    QString albumTitle = player->metaData(QMediaMetaData::AlbumTitle).toString();
    QString trackYear = player->metaData(QMediaMetaData::Year).toString();

    if (artist.isEmpty()) {
        artist = "Неизвестен";
    }
    if (title.isEmpty()) {
        title = "Без названия";
    }
    if (albumTitle.isEmpty()) {
        albumTitle = "Неизвестный альбом";
    }
    if (!trackYear.isEmpty()) {
        albumTitle += " (" + trackYear + ")";
    }

    trackTitleLabel->setText(artist + " - " + title);
    albumTitleLabel->setText(albumTitle);

    qDebug() << player->availableMetaData();

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
            // Если папка — находим все аудиофайлы
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
            // Если файл — проверяем расширение
            QString ext = fileInfo.suffix().toLower();
            if (allowedExtensions.contains(ext)) {
                newFiles << fileInfo.absoluteFilePath();
            }
        }
    }
    if (!newFiles.isEmpty()) {
        playlistFiles = newFiles;
        trackListWidget->clear();
        for (const QString &filePath : playlistFiles) {
            QFileInfo fi(filePath);
            trackListWidget->addItem(fi.fileName());
        }
        playTrack(0);
    }
}

void AudioPlayer::playTrack(int index)
{
    if (index >= 0 && index < playlistFiles.size()) {
        currentTrackIndex = index;
        QString filePath = playlistFiles.at(index);
        player->setMedia(QUrl::fromLocalFile(filePath));
        player->play();

        // Подсвечиваем текущий трек
        trackListWidget->setCurrentRow(index);
    }
}

void AudioPlayer::playNext()
{
    if (playlistFiles.isEmpty()) return;

    if (repeatMode == RepeatOne) {
        player->setMedia(QUrl::fromLocalFile(playlistFiles[currentTrackIndex]));
        player->play();
        return;
    }

    if (shuffleMode) {
        int nextIndex = QRandomGenerator::global()->bounded(playlistFiles.size());
        currentTrackIndex = nextIndex;
    } else {
        currentTrackIndex++;
        if (currentTrackIndex >= playlistFiles.size()) {
            if (repeatMode == RepeatAll)
                currentTrackIndex = 0;
            else
                return; // стоп, если NoRepeat
        }
    }

    player->setMedia(QUrl::fromLocalFile(playlistFiles[currentTrackIndex]));
    player->play();
    trackListWidget->setCurrentRow(currentTrackIndex);
}

void AudioPlayer::playPrevious()
{
    if (playlistFiles.isEmpty()) return;
    int prevIndex = (currentTrackIndex - 1 + playlistFiles.size()) % playlistFiles.size();
    playTrack(prevIndex);
}

void AudioPlayer::toggleRepeatMode()
{
    repeatMode = static_cast<RepeatMode>((repeatMode + 1) % 3);

    switch (repeatMode) {
    case NoRepeat:
        repeatButton->setText(" выкл");
        break;
    case RepeatAll:
        repeatButton->setText(" все");
        break;
    case RepeatOne:
        repeatButton->setText(" трек");
        break;
    }
}

void AudioPlayer::toggleShuffleMode()
{
    shuffleMode = !shuffleMode;
    shuffleButton->setText(shuffleMode ? "🔀 вкл" : "🔀 выкл");
}

void AudioPlayer::applyStyles() {
    QFile file(":/css/audioplayer.qss");

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        this->setStyleSheet(styleSheet);
    } else {
        qDebug() << "Ошибка при открытии файла стилей!";
    }
}
