#include "dbmanager.h"
#include <QSqlError>
#include <QVariant>
#include <QDebug>

DBManager::DBManager(QObject *parent) : QObject(parent) {}

bool DBManager::init(const QString &dbPath)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "Failed to open database: " << db.lastError();
        return false;
    }

    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON");

    query.exec("CREATE TABLE IF NOT EXISTS playlists ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT NOT NULL)");

    query.exec("CREATE TABLE IF NOT EXISTS playlist_tracks ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "playlist_id INTEGER NOT NULL,"
               "track_path TEXT NOT NULL,"
               "track_order INTEGER,"
               "FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE)");

    return true;
}

int DBManager::createPlaylist(const QString &name)
{
    QSqlQuery query;
    query.prepare("INSERT INTO playlists (name) VALUES (:name)");
    query.bindValue(":name", name);
    if (query.exec()) {
        return query.lastInsertId().toInt();
    } else {
        qWarning() << "createPlaylist error:" << query.lastError();
        return -1;
    }
}

void DBManager::deletePlaylist(int playlistId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM playlists WHERE id = :id");
    query.bindValue(":id", playlistId);
    query.exec();
}

void DBManager::renamePlaylist(int playlistId, const QString &newName)
{
    QSqlQuery query;
    query.prepare("UPDATE playlists SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", playlistId);
    query.exec();
}

QStringList DBManager::getAllPlaylists()
{
    QStringList list;
    QSqlQuery query("SELECT name FROM playlists ORDER BY name");
    while (query.next()) {
        list << query.value(0).toString();
    }
    return list;
}

QMap<int, QString> DBManager::getAllPlaylistsWithIds()
{
    QMap<int, QString> playlists;
    QSqlQuery query("SELECT id, name FROM playlists ORDER BY name");
    while (query.next()) {
        playlists.insert(query.value(0).toInt(), query.value(1).toString());
    }
    return playlists;
}

void DBManager::addTrackToPlaylist(int playlistId, const QString &trackPath, int order)
{
    QSqlQuery query;
    query.prepare("INSERT INTO playlist_tracks (playlist_id, track_path, track_order) "
                  "VALUES (:pid, :path, :order)");
    query.bindValue(":pid", playlistId);
    query.bindValue(":path", trackPath);
    query.bindValue(":order", order);

    if (!query.exec()) {
        qWarning() << "Не удалось добавить трек в плейлист:" << query.lastError();
    }
}

void DBManager::clearPlaylistTracks(int playlistId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM playlist_tracks WHERE playlist_id = :pid");
    query.bindValue(":pid", playlistId);
    query.exec();
}

QStringList DBManager::loadPlaylistTracks(int playlistId)
{
    QStringList tracks;
    QSqlQuery query;
    query.prepare("SELECT track_path FROM playlist_tracks WHERE playlist_id = :pid ORDER BY track_order");
    query.bindValue(":pid", playlistId);
    query.exec();

    while (query.next()) {
        tracks << query.value(0).toString();
    }
    return tracks;
}

int DBManager::getTrackCountInPlaylist(int playlistId)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM playlist_tracks WHERE playlist_id = ?");
    query.addBindValue(playlistId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    qWarning() << "Ошибка при получении количества треков в плейлисте:" << query.lastError();
    return 0;
}
