#include <QApplication>
#include "audioplayer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    AudioPlayer player;
    player.resize(1000, 500);
    player.show();

    return app.exec();
}
