#include "trackmanager.h"
#include <QRandomGenerator>

TrackManager::TrackManager(QObject *parent)
    : QObject(parent)
{
}

void TrackManager::setPlaylist(const QStringList &files)
{
    playlist = files;
    currentTrackIndex = -1;
    emit playlistUpdated(playlist);
}

QStringList TrackManager::getPlaylist()
{
    return playlist;
}


QString TrackManager::currentTrack() const
{
    if (currentTrackIndex >= 0 && currentTrackIndex < playlist.size()) {
        return playlist[currentTrackIndex];
    }
    return QString();
}

bool TrackManager::hasTracks() const
{
    return !playlist.isEmpty();
}

void TrackManager::playTrack(int index)
{
    if (index >= 0 && index < playlist.size()) {
        currentTrackIndex = index;
        emit trackChanged(playlist.at(currentTrackIndex), currentTrackIndex);
    }
}

void TrackManager::playNext()
{
    if (playlist.isEmpty()) return;

    if (repeat == RepeatOne) {
        emit trackChanged(playlist.at(currentTrackIndex), currentTrackIndex);
        return;
    }

    if (shuffle) {
        int nextIndex = QRandomGenerator::global()->bounded(playlist.size());
        currentTrackIndex = nextIndex;
    } else {
        currentTrackIndex++;
        if (currentTrackIndex >= playlist.size()) {
            if (repeat == RepeatAll)
                currentTrackIndex = 0;
            else
                return;
        }
    }

    emit trackChanged(playlist.at(currentTrackIndex), currentTrackIndex);
}

void TrackManager::playPrevious()
{
    if (playlist.isEmpty()) return;

    int prevIndex = (currentTrackIndex - 1 + playlist.size()) % playlist.size();
    playTrack(prevIndex);
}

void TrackManager::toggleShuffle()
{
    shuffle = !shuffle;
}

void TrackManager::toggleRepeat()
{
    repeat = static_cast<RepeatMode>((repeat + 1) % 3);
}

bool TrackManager::shuffleEnabled() const
{
    return shuffle;
}

TrackManager::RepeatMode TrackManager::repeatMode() const
{
    return repeat;
}

int TrackManager::currentIndex() const
{
    return currentTrackIndex;
}

QStringList TrackManager::allTracks() const
{
    return playlist;
}
