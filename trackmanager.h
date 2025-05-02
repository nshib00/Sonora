#pragma once

#include <QStringList>
#include <QObject>

class TrackManager : public QObject
{
    Q_OBJECT

public:
    enum RepeatMode { NoRepeat, RepeatAll, RepeatOne };

    explicit TrackManager(QObject *parent = nullptr);

    void setPlaylist(const QStringList &files);
    QString currentTrack() const;
    bool hasTracks() const;

    void playTrack(int index);
    void playNext();
    void playPrevious();
    void toggleShuffle();
    void toggleRepeat();

    bool shuffleEnabled() const;
    RepeatMode repeatMode() const;
    int currentIndex() const;
    QStringList allTracks() const;

signals:
    void trackChanged(const QString &filePath, int index);
    void playlistUpdated(const QStringList &files);

private:
    QStringList playlist;
    int currentTrackIndex = -1;
    bool shuffle = false;
    RepeatMode repeat = NoRepeat;
};
