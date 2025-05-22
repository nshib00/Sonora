#ifndef TRACKLISTMODEL_H
#define TRACKLISTMODEL_H

#include <QAbstractListModel>
#include <QFileInfo>

class TrackListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TrackListModel(QObject *parent = nullptr);

    // Core overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Data operations
    void setTrackList(const QStringList &tracks);
    QString trackPathAt(int index) const;
    void removeTrackAt(int index);
    QStringList getTrackList() const;

private:
    QStringList trackPaths;
};

#endif // TRACKLISTMODEL_H
