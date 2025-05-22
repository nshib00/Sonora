#include "playlistmodel.h"
#include <QFileInfo>

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_tracks.count();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tracks.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        QFileInfo fileInfo(m_tracks.at(index.row()));
        return fileInfo.fileName();
    }

    return QVariant();
}

void PlaylistModel::setTracks(const QStringList &tracks)
{
    beginResetModel();
    m_tracks = tracks;
    endResetModel();
}

void PlaylistModel::addTrack(const QString &trackPath)
{
    beginInsertRows(QModelIndex(), m_tracks.count(), m_tracks.count());
    m_tracks << trackPath;
    endInsertRows();
}

void PlaylistModel::removeTrack(int row)
{
    if (row < 0 || row >= m_tracks.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_tracks.removeAt(row);
    endRemoveRows();
}

void PlaylistModel::clear()
{
    beginResetModel();
    m_tracks.clear();
    endResetModel();
}

QStringList PlaylistModel::getTrackList() const
{
    return m_tracks;
}

QString PlaylistModel::getTrack(int row) const
{
    if (row < 0 || row >= m_tracks.size())
        return QString();
    return m_tracks.at(row);
}
