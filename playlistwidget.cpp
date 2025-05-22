#include "playlistwidget.h"
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>

PlaylistWidget::PlaylistWidget(DBManager *db, QWidget *parent)
    : QWidget(parent), dbManager(db)
{
    listWidget = new QListWidget(this);
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    addButton = new QPushButton("Добавить плейлист", this);

    layout = new QVBoxLayout(this);
    layout->addWidget(addButton);
    layout->addWidget(listWidget);

    connect(addButton, &QPushButton::clicked, this, &PlaylistWidget::onAddPlaylist);
    connect(listWidget, &QListWidget::customContextMenuRequested, this, &PlaylistWidget::onItemContextMenu);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &PlaylistWidget::onItemDoubleClicked);

    loadPlaylistsFromDB();
}

void PlaylistWidget::loadPlaylistsFromDB()
{
    listWidget->clear();
    itemToIdMap.clear();

    QMap<int, QString> playlists = dbManager->getAllPlaylistsWithIds();
    for (auto it = playlists.begin(); it != playlists.end(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(it.value());
        listWidget->addItem(item);
        itemToIdMap[item] = it.key();
    }
}

void PlaylistWidget::onAddPlaylist()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Новый плейлист", "Введите имя:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        int newId = dbManager->createPlaylist(name);
        if (newId != -1) {
            QListWidgetItem *item = new QListWidgetItem(name);
            listWidget->addItem(item);
            itemToIdMap[item] = newId;
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось создать плейлист.");
        }
    }
}

void PlaylistWidget::onAddTrackToPlaylist()
{
    QListWidgetItem *item = listWidget->currentItem();
    if (!item || !itemToIdMap.contains(item)) return;

    int playlistId = itemToIdMap.value(item);
    QString filePath = QFileDialog::getOpenFileName(this, "Добавить трек", "", "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a)");

    if (!filePath.isEmpty()) {
        // Получаем количество уже добавленных треков для этого плейлиста
        int order = dbManager->getTrackCountInPlaylist(playlistId) + 1;

        // Добавляем трек в плейлист с порядковым номером
        dbManager->addTrackToPlaylist(playlistId, filePath, order);

        // Обновляем плейлист в интерфейсе
        loadTracksForPlaylist(playlistId);
    }
}



void PlaylistWidget::onItemContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = listWidget->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction *renameAction = menu.addAction("Переименовать");
    QAction *deleteAction = menu.addAction("Удалить");
    QAction *addTrackAction = menu.addAction("Добавить трек");

    QAction *selectedAction = menu.exec(listWidget->mapToGlobal(pos));
    if (selectedAction == renameAction) {
        listWidget->setCurrentItem(item);
        onRenamePlaylist();
    } else if (selectedAction == deleteAction) {
        listWidget->setCurrentItem(item);
        onDeletePlaylist();
    } else if (selectedAction == addTrackAction) {
        listWidget->setCurrentItem(item);
        onAddTrackToPlaylist();
    }
}


void PlaylistWidget::onRenamePlaylist()
{
    QListWidgetItem *item = listWidget->currentItem();
    if (!item) return;

    int id = itemToIdMap.value(item);
    bool ok;
    QString newName = QInputDialog::getText(this, "Переименовать плейлист", "Новое имя:",
                                            QLineEdit::Normal, item->text(), &ok);
    if (ok && !newName.isEmpty()) {
        dbManager->renamePlaylist(id, newName);
        item->setText(newName);
    }
}

void PlaylistWidget::onDeletePlaylist()
{
    QListWidgetItem *item = listWidget->currentItem();
    if (!item) return;

    int id = itemToIdMap.value(item);
    if (QMessageBox::question(this, "Удаление плейлиста",
                              "Удалить этот плейлист и все его треки?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        dbManager->deletePlaylist(id);
        itemToIdMap.remove(item);
        delete item;
    }
}

void PlaylistWidget::onItemDoubleClicked(QListWidgetItem *item)
{
    if (!item || !itemToIdMap.contains(item)) return;
    int id = itemToIdMap[item];
    emit playlistSelected(id, item->text());
}

void PlaylistWidget::loadTracksForPlaylist(int playlistId)
{
    listWidget->clear();
    QStringList tracks = dbManager->loadPlaylistTracks(playlistId);

    for (const QString &trackPath : tracks) {
        QFileInfo fileInfo(trackPath);
        listWidget->addItem(fileInfo.fileName());
    }
}

