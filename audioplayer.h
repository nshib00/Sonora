#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QFileDialog>
#include <QDir>
#include <QFileInfoList>
#include <QListWidget>
#include <QFile>
#include <QTextStream>

#include "trackmanager.h"

class QPushButton;
class QSlider;
class QLabel;

class AudioPlayer : public QWidget
{
    Q_OBJECT

public:
    AudioPlayer(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void openFile();
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void setPosition(int position);
    void updateTimeLabel();
    void updateMetaData();
    void openFolder();
    void applyStyles();

private:
    void setupUi();
    void setupConnections();

    QMediaPlayer *player;
    TrackManager *trackManager;

    QPushButton *openButton;
    QPushButton *openFolderButton;
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;

    QListWidget *trackListWidget;
    QPushButton *prevButton;
    QPushButton *nextButton;

    QPushButton *repeatButton;
    QPushButton *shuffleButton;
    enum RepeatMode { NoRepeat, RepeatAll, RepeatOne };
    RepeatMode repeatMode = NoRepeat;
    bool shuffleMode = false;

    QStringList playlistFiles;
    int currentTrackIndex = -1;

    void playTrack(int index);

    QSlider *progressSlider;
    QSlider *volumeSlider;

    QLabel *timeLabel;
    QLabel *trackTitleLabel;
    QLabel *albumTitleLabel;
    QLabel *coverArtLabel;

    qint64 duration = 0;
};

#endif
