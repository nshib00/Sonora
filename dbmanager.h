#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = nullptr);

    bool init(const QString &dbPath);

    int createPlaylist(const QString &name);
    void deletePlaylist(int playlistId);
    void renamePlaylist(int playlistId, const QString &newName);
    QStringList getAllPlaylists();                    // список названий
    QMap<int, QString> getAllPlaylistsWithIds();      // id -> name

    void addTrackToPlaylist(int playlistId, const QString &trackPath, int order);
    void clearPlaylistTracks(int playlistId);
    QStringList loadPlaylistTracks(int playlistId);
    int getTrackCountInPlaylist(int playlistId);

private:
    QSqlDatabase db;
};

#endif // DBMANAGER_H
