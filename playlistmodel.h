#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PlaylistModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Управление треками
    void setTracks(const QStringList &tracks);
    void addTrack(const QString &trackPath);
    void removeTrack(int row);
    void clear();

    QStringList getTrackList() const;
    QString getTrack(int row) const;

private:
    QStringList m_tracks;
};

#endif // PLAYLISTMODEL_H
