#include "tracklistmodel.h"

TrackListModel::TrackListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TrackListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return trackPaths.size();
}

QVariant TrackListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= trackPaths.size())
        return QVariant();

    const QString &filePath = trackPaths.at(index.row());
    QFileInfo fi(filePath);

    switch (role) {
    case Qt::DisplayRole:
        return fi.fileName();
    case Qt::ToolTipRole:
        return filePath;
    case Qt::UserRole:
        return filePath;
    default:
        return QVariant();
    }
}

Qt::ItemFlags TrackListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void TrackListModel::setTrackList(const QStringList &tracks)
{
    beginResetModel();
    trackPaths = tracks;
    endResetModel();
}

QString TrackListModel::trackPathAt(int index) const
{
    if (index < 0 || index >= trackPaths.size())
        return QString();
    return trackPaths.at(index);
}

void TrackListModel::removeTrackAt(int index)
{
    if (index < 0 || index >= trackPaths.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    trackPaths.removeAt(index);
    endRemoveRows();
}

QStringList TrackListModel::getTrackList() const
{
    return trackPaths;
}
