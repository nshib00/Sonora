#include <QApplication>
#include "audioplayer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/icons/logo.ico"));

    AudioPlayer player;
    player.resize(1400, 700);
    player.show();

    return app.exec();
}
