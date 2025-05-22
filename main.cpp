#include <QApplication>
#include <QIcon>
#include <QFile>

#include "audioplayer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/icons/logo.ico"));

    QFile file(":/css/audioplayer.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        app.setStyleSheet(stream.readAll());
    }

    AudioPlayer player;
    player.resize(1400, 700);
    player.show();

    return app.exec();
}
