#include "videoplayer.h"
#include "audioplayer.h"
#include <stdio.h>
#include <QDateTime>
#include <QDebug>

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}

#endif

QMutex mutex;


VideoPlayer::VideoPlayer()
{
    audio_timeStamp = 0;
    decode_time_count = 0;
    Display_timer = new QTimer(this);
    connect(Display_timer, SIGNAL(timeout()), this, SLOT(decode_timer_timeout()));
    Display_timer->start(1);
}

VideoPlayer::~VideoPlayer()
{

}

void VideoPlayer::startPlay()
{
    ///调用 QThread 的start函数 将会自动执行下面的run函数 run函数是一个新的线程
    this->start();
}

void VideoPlayer::run()
{
    //char *file_path = mFileName.toUtf8().data();//这样转换有问题
    //char *file_path = mFileName.toLatin1().data();//这样转换有奇怪的问题
    QByteArray ba = mFileName.toLatin1();
    char *file_path = ba.data();

    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    AVPacket *packet;
    uint8_t *out_buffer;

    static struct SwsContext *img_convert_ctx;

    int videoStream, i, numBytes;
    int ret, got_picture;

    avcodec_register_all();// 必须的

    av_register_all(); //初始化FFMPEG调用了这个才能正常适用编码器和解码器

    avformat_network_init();

    //Allocate an AVFormatContext.
    pFormatCtx = avformat_alloc_context();
    AVDictionary *options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);//选择使用传输方式
    if (avformat_open_input(&pFormatCtx, file_path, NULL, &options) != 0)
    {
        qDebug("can't open the file. \n");
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug("Could't find stream infomation.\n");
        return;
    }

    videoStream = -1;

    //循环查找视频中包含的流信息，直到找到视频类型的流
    //便将其记录下来 保存到videoStream变量中
    //这里我们现在只处理视频流  音频流先不管他
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
        }
    }

    //如果videoStream为-1 说明没有找到视频流
    if (videoStream == -1)
    {
        qDebug("Didn't find a video stream.\n");
        return;
    }

    //获取帧率，按帧率播放
#if 1
    qDebug()<<"r_frame_rate"<<pFormatCtx->streams[videoStream]->r_frame_rate.num;
    qDebug()<<"r_frame_rate"<<pFormatCtx->streams[videoStream]->r_frame_rate.den;
    float frame_rate = (float)pFormatCtx->streams[videoStream]->r_frame_rate.num/pFormatCtx->streams[videoStream]->r_frame_rate.den;
    qDebug()<<frame_rate;
    int frame_rate2 = qRound(frame_rate);//四舍五入操作
    qDebug()<<frame_rate2;
    float display_internal = (float)1000/frame_rate;
    qDebug()<<display_internal;
    int display_internal2 = qRound(display_internal);
    qDebug()<<display_internal2;


    display_internal = display_internal2;

#else
    int display_internal = 0;

#endif
    //查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if (pCodec == NULL)
    {
        qDebug("Codec not found.\n");
        return;
    }

    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        qDebug("Could not open codec.\n");
        return;
    }

    pFrame = avcodec_alloc_frame();
    pFrameRGB = avcodec_alloc_frame();

    //这里我们改成了 将解码后的YUV数据转换成RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

    numBytes = avpicture_get_size(PIX_FMT_RGB32, pCodecCtx->width,pCodecCtx->height);

    out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer, PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);

    int y_size = pCodecCtx->width * pCodecCtx->height;

    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个packet
    av_new_packet(packet, y_size); //分配packet的数据

    av_dump_format(pFormatCtx, 0, file_path, 0); //输出视频信息

    while (1)
    {
        //qDebug()<<"run: thread ID is "<<currentThreadId();
        //qDebug()<<"start decode"<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:zzz");
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            break; //这里认为视频读取完了
        }

        if (packet->stream_index == videoStream)
        {
            static double video_old_timeStamp = 0;
            /*计算播放的PTS*/
            AVStream *st = *(pFormatCtx->streams);
            double video_new_timeStamp = packet->pts * av_q2d(st->time_base);
            //qDebug()<<"Video pts is"<<packet->pts;
            qDebug()<<"Video pts is"<<video_new_timeStamp;

            double delay = video_new_timeStamp - video_old_timeStamp;//上一帧的pts减去当前帧的pts得到一个延迟时间
            video_old_timeStamp = video_new_timeStamp;

            mutex.lock();
            double diff = video_new_timeStamp - audio_timeStamp;
            qDebug()<<"Audio pts is "<<audio_timeStamp;
            mutex.unlock();

            //qDebug()<<"diff is"<<diff;
            if(diff > 0.05)//快了相差5毫秒才认为是快了
            {
                //delay *= 2;
                display_internal2 = display_internal * 2;
                //qDebug()<<"fast";
            }
            else if(diff < -0.05)//
            {
                //delay = 0;
                display_internal2 = 0;
                //qDebug()<<"slow";
            }
            else
            {
                display_internal2 = display_internal;
                //qDebug()<<"sync";
            }

            //qDebug()<<"display_internal is"<<display_internal;
            //qDebug()<<"display_internal2 is"<<display_internal2;
            /*解码*/
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0)
            {
                qDebug("decode error.\n");
                return;
            }

            if (got_picture)
            {
                sws_scale(img_convert_ctx,
                          (uint8_t const * const *) pFrame->data,
                          pFrame->linesize,
                          0,
                          pCodecCtx->height,
                          pFrameRGB->data,
                          pFrameRGB->linesize);


                //把这个RGB数据 用QImage加载
                QImage tmpImg((uchar *)out_buffer,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
                QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示

                while(decode_time_count < (display_internal2));
                //while(decode_time_count < (40));

                //qDebug()<<decode_time_count<<"ms";
                decode_time_count = 0;
                emit sig_GetOneFrame(image);  //发送信号

                //qDebug()<<"finish decode"<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:zzz");
            }
        }
        av_free_packet(packet);
    }
    av_free(out_buffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}

void VideoPlayer::decode_timer_timeout()
{
    decode_time_count++;
}

void VideoPlayer::get_audio_clock(double clock)
{
    mutex.lock();
    audio_timeStamp = clock;
    mutex.unlock();
    //qDebug()<<"get_audio_clock: thread ID is "<<currentThreadId();//这个也是主线程ID
}


