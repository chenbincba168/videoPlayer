#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QThread>
#include <QImage>
#include <QTimer>
#include <QMutex>

extern QMutex mutex;

class VideoPlayer : public QThread
{
    Q_OBJECT

public:
    explicit VideoPlayer();
    ~VideoPlayer();
    void setFileName(QString path){mFileName = path;}
    void startPlay();

signals:
    void sig_GetOneFrame(QImage); //没获取到一帧图像 就发送此信号

protected:
    void run();

private:
    QString mFileName;
    int decode_time_count;
    QTimer *Display_timer;
    double audio_timeStamp;
private slots:
    void decode_timer_timeout();
    void get_audio_clock(double clock);
};

#endif // VIDEOPLAYER_H
