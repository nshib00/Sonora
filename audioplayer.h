#include "trackmanager.h"
#include <QMediaPlayer>
#include "audioplayerui.h"

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
    void openFolder();
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void setPosition(int position);
    void updateTimeLabels();
    void updateMetaData();
    void showTrackContextMenu(const QPoint &pos);

private:
    void setupConnections();
    void displayPlaylistTracks(const QStringList &tracks, int playlistId);
    void applyStyles();

    QMediaPlayer *player;
    TrackManager *trackManager;
    DBManager *dbManager;

    AudioPlayerUI *ui;

    QStringList playlistFiles;
    int currentTrackIndex = -1;
    qint64 duration = 0;

    void playTrack(int index);
};
