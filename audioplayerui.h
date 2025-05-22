#ifndef AUDIOPLAYERUI_H
#define AUDIOPLAYERUI_H

#include <QWidget>

class QPushButton;
class QSlider;
class QLabel;
class QListWidget;
class PlaylistWidget;
class DBManager;

class AudioPlayerUI : public QWidget
{
    Q_OBJECT

public:
    explicit AudioPlayerUI(DBManager* dbManager, QWidget *parent = nullptr);

    QPushButton *openButton;
    QPushButton *openFolderButton;
    QPushButton *playPauseButton;
    QPushButton *stopButton;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QPushButton *repeatButton;
    QPushButton *shuffleButton;

    QSlider *progressSlider;
    QSlider *volumeSlider;

    QLabel *leftTimeLabel;
    QLabel *rightTimeLabel;
    QLabel *trackTitleLabel;
    QLabel *albumTitleLabel;
    QLabel *coverArtLabel;

    QListWidget *trackListWidget;
    PlaylistWidget *playlistWidget;

    QIcon loadColoredIcon(const QString &resourcePath, const QColor &color);
private:
    void setupUi(DBManager* dbManager);

};

#endif // AUDIOPLAYERUI_H
