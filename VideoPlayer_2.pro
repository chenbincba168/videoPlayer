#-------------------------------------------------
#
# Project created by QtCreator 2016-09-01T16:10:47
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoPlayer_2
TEMPLATE = app


SOURCES += src/main.cpp \
    src/videoplayer.cpp \
    src/mainwindow.cpp \
    src/audioplayer.cpp \
    src/glwidget.cpp

HEADERS  += \
    src/videoplayer.h \
    src/mainwindow.h \
    src/audioplayer.h \
    src/glwidget.h


FORMS    += \
    src/mainwindow.ui

INCLUDEPATH += $$PWD/ffmpeg/include \
                $$PWD/src

LIBS += $$PWD/ffmpeg/lib/avcodec.lib \
        $$PWD/ffmpeg/lib/avdevice.lib \
        $$PWD/ffmpeg/lib/avfilter.lib \
        $$PWD/ffmpeg/lib/avformat.lib \
        $$PWD/ffmpeg/lib/avutil.lib \
        $$PWD/ffmpeg/lib/postproc.lib \
        $$PWD/ffmpeg/lib/swresample.lib \
        $$PWD/ffmpeg/lib/swscale.lib \
        $$PWD/ffmpeg/lib/SDL2.lib

