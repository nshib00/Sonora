#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMap>
#include "dbmanager.h"

class PlaylistWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistWidget(DBManager *db, QWidget *parent = nullptr);

signals:
    void playlistSelected(int id, const QString &name);
    void trackAddedToPlaylist(int playlistId);

private slots:
    void onAddPlaylist();
    void onItemContextMenu(const QPoint &pos);
    void onRenamePlaylist();
    void onDeletePlaylist();
    void onItemDoubleClicked(QListWidgetItem *item);

private:
    QListWidget *listWidget;
    QPushButton *addButton;
    QVBoxLayout *layout;

    DBManager *dbManager;
    QMap<QListWidgetItem*, int> itemToIdMap;

    void loadPlaylistsFromDB();
    void loadTracksForPlaylist(int playlistId);
    void onAddTrackToPlaylist();
    void onAddFolderToPlaylist();
};

#endif // PLAYLISTWIDGET_H
