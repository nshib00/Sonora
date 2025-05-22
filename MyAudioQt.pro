QT       += core gui multimedia svg sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audioplayer.cpp \
    dbmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    trackmanager.cpp

HEADERS += \
    audioplayer.h \
    dbmanager.h \
    mainwindow.h \
    trackmanager.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    css/audioplayer.qss \
    fonts/Rubik-Light.ttf \
    fonts/Rubik-Medium.ttf \
    fonts/Rubik-Regular.ttf \
    images/icons/file.svg \
    images/icons/folder.svg \
    images/icons/logo.ico \
    images/icons/pause.svg \
    images/icons/play.svg \
    images/icons/refresh-ccw.svg \
    images/icons/shuffle.svg \
    images/icons/skip-back.svg \
    images/icons/skip-forward.svg \
    images/icons/stop.svg \
    images/placeholder.png
