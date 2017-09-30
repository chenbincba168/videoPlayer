#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QThread>
#include <QImage>
#include <QTimer>
#include <QMutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "sdl2/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

//Output PCM
#define OUTPUT_PCM 1
//Use SDL
#define USE_SDL 1


#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;

extern double audio_clock;

class AudioPlayer : public QThread
{
    Q_OBJECT

public:
    explicit AudioPlayer();
    ~AudioPlayer();
    void setFileName(QString path){mFileName = path;}
    void startPlay();

signals:
    void send_audio_clock(double);
protected:
    void run();

private:
    QString mFileName;
private slots:

};

#endif // AUDIOPLAYER_H
