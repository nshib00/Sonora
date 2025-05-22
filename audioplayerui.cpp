#include "audioplayerui.h"
#include "playlistwidget.h"
#include "dbmanager.h"

#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QListWidget>
#include <QStyle>
#include <QDebug>

AudioPlayerUI::AudioPlayerUI(DBManager* dbManager, QWidget *parent)
    : QWidget(parent)
{
    setupUi(dbManager);
    applyStyles();
}

void AudioPlayerUI::setupUi(DBManager* dbManager)
{
    openButton = new QPushButton;
    openButton->setIcon(loadColoredIcon(":/images/icons/file.svg", Qt::white));

    openFolderButton = new QPushButton;
    openFolderButton->setIcon(loadColoredIcon(":/images/icons/folder.svg", Qt::white));

    playPauseButton = new QPushButton;
    playPauseButton->setIcon(loadColoredIcon(":/images/icons/play.svg", Qt::white));

    stopButton = new QPushButton;
    stopButton->setIcon(loadColoredIcon(":/images/icons/stop.svg", Qt::white));

    prevButton = new QPushButton;
    prevButton->setIcon(loadColoredIcon(":/images/icons/skip-back.svg", Qt::white));

    nextButton = new QPushButton;
    nextButton->setIcon(loadColoredIcon(":/images/icons/skip-forward.svg", Qt::white));

    repeatButton = new QPushButton;
    repeatButton->setIcon(loadColoredIcon(":/images/icons/refresh-ccw.svg", Qt::white));

    shuffleButton = new QPushButton;
    shuffleButton->setIcon(loadColoredIcon(":/images/icons/shuffle.svg", Qt::white));
    shuffleButton->setText(" выкл");

    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 0);

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);

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
    trackListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    playlistWidget = new PlaylistWidget(dbManager, this);
    playlistWidget->setMinimumWidth(200);
    playlistWidget->setMaximumWidth(250);

    // Layout
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

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(playlistWidget);
    mainLayout->addLayout(audioPlayerLayout);
    mainLayout->addWidget(trackListWidget);

    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

void AudioPlayerUI::applyStyles()
{
    QFile file(":/css/audioplayer.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        this->setStyleSheet(stream.readAll());
    } else {
        qDebug() << "Не удалось загрузить стили.";
    }
}

QIcon AudioPlayerUI::loadColoredIcon(const QString &resourcePath, const QColor &color)
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
