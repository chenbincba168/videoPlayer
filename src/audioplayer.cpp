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

double audio_clock = 0;

AudioPlayer::AudioPlayer()
{

}

AudioPlayer::~AudioPlayer()
{

}

void fill_audio(void *udata,Uint8 *stream,int len)
{
    //SDL 2.0
    SDL_memset(stream, 0, len);
    if(audio_len==0)		/*  Only  play  if  we  have  data  left  */
        return;
    len=(len>audio_len?audio_len:len);	/*  Mix  as  much  data  as  possible  */

    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

void AudioPlayer::startPlay()
{
    this->start();
}

void AudioPlayer::run()
{
    AVFormatContext	*pFormatCtx;
    int				i, audioStream;
    AVCodecContext	*pCodecCtx;
    AVCodec			*pCodec;
    AVPacket		*packet;
    uint8_t			*out_buffer;
    AVFrame			*pFrame;
    SDL_AudioSpec wanted_spec;
    int ret;
    uint32_t len = 0;
    int got_audio;
    int index = 0;
    int64_t in_channel_layout;
    struct SwrContext *au_convert_ctx;

    FILE *pFile=NULL;
    QByteArray ba = mFileName.toLatin1();
    char *url = ba.data();

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx,url,NULL,NULL)!=0)//Open
    {
        qDebug("Couldn't open input stream.\n");
        exit(1);
    }

    if(avformat_find_stream_info(pFormatCtx,NULL)<0)// Retrieve stream information
    {
        qDebug("Couldn't find stream information.\n");
        exit(1);
    }

    av_dump_format(pFormatCtx, 0, url, false);// Dump valid information onto standard error
    audioStream = -1; // Find the first audio stream

    for(i=0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStream = i;
            break;
        }
    }

    if(audioStream == -1)
    {
        qDebug("Didn't find a audio stream.\n");
        exit(1);
    }

    pCodecCtx=pFormatCtx->streams[audioStream]->codec;// Get a pointer to the codec context for the audio stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id); // Find the decoder for the audio stream
    if(pCodec==NULL)
    {
        qDebug("Codec not found.\n");
        exit(1);
    }

    if(avcodec_open2(pCodecCtx, pCodec,NULL) < 0) // Open codec
    {
        qDebug("Could not open codec.\n");
        exit(1);
    }

#if OUTPUT_PCM
    pFile=fopen("output.pcm", "wb");
#endif

    packet=(AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);

    uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;//Out Audio Param
    int out_nb_samples=pCodecCtx->frame_size; //nb_samples: AAC-1024 MP3-1152
    AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;

    //int out_sample_rate=44100;
    int out_sample_rate = pCodecCtx->sample_rate;//获取采样率
    int out_channels=av_get_channel_layout_nb_channels(out_channel_layout);
    int out_buffer_size=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);//Out Buffer Size
    out_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
    pFrame=av_frame_alloc();

    /*SDL*/
#if USE_SDL
    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))//Init
    {
        qDebug( "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    /*SDL_AudioSpec*/
    wanted_spec.freq = out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = out_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = out_nb_samples;
    wanted_spec.callback = fill_audio;
    wanted_spec.userdata = pCodecCtx;

    if (SDL_OpenAudio(&wanted_spec, NULL)<0)
    {
        qDebug("can't open audio.\n");
        exit(1);
    }
#endif

    in_channel_layout=av_get_default_channel_layout(pCodecCtx->channels); //FIX:Some Codec's Context Information is missing
    au_convert_ctx = swr_alloc();//Swr
    au_convert_ctx=swr_alloc_set_opts(au_convert_ctx,out_channel_layout,
                                      out_sample_fmt, out_sample_rate,
                                      in_channel_layout,pCodecCtx->sample_fmt ,
                                      pCodecCtx->sample_rate,
                                      0,
                                      NULL);
    swr_init(au_convert_ctx);

    while(av_read_frame(pFormatCtx, packet)>=0)
    {
        if(packet->stream_index == audioStream)
        {
            /*计算audio clock*/
            AVStream *st = *pFormatCtx->streams;

#if 0
//            double pts = 0;
//            if (packet->pts != AV_NOPTS_VALUE)
//            {
//                audio_clock = av_q2d(st->time_base) * packet->pts;
//                pts = audio_clock;
//                qDebug()<<"calc audio pts 1";

//            }
//            else
//            {
//                int hw_buf_size = audio_len;
//                int bytes_per_sec = pCodecCtx->sample_rate * pCodecCtx->channels * 2;
//                pts = audio_clock - double(hw_buf_size) / bytes_per_sec;
//                qDebug()<<"calc audio pts 2";
//            }
//            double pts = 0;
//            double rescale_last_pts = 0;
//            if (rescale_last_pts == AV_NOPTS_VALUE)
//            {
//                rescale_last_pts = 0;
//            }
//            decoded_frame->pts = rescale_last_pts;
//            rescale_last_pts += decoded_frame->nb_samples; // duration
//            emit send_audio_clock(pts);
#else
            audio_clock = av_q2d(st->time_base) * packet->pts;
            double pts = audio_clock;
            emit send_audio_clock(pts);
            //qDebug()<<"Audio pts is "<<pts;

#endif
            ret = avcodec_decode_audio4( pCodecCtx, pFrame,&got_audio, packet);
            if ( ret < 0 )
            {
                qDebug("Error in decoding audio frame.\n");
                exit(1);
            }
#if 0
            double pts = 0;
            static int rescale_last_pts = 0;
            if (rescale_last_pts == AV_NOPTS_VALUE)
            {
                rescale_last_pts = 0;
            }
            pFrame->pts = rescale_last_pts;
            rescale_last_pts += pFrame->nb_samples; // duration
            pts = av_q2d(st->time_base) * rescale_last_pts;
            emit send_audio_clock(pts);

#endif
            if ( got_audio > 0 )
            {
                swr_convert(au_convert_ctx,&out_buffer, MAX_AUDIO_FRAME_SIZE,(const uint8_t **)pFrame->data , pFrame->nb_samples);
#if 1
                //qDebug("Audio index:%5d\t pts:%lld\t packet size:%d\n",index,packet->pts,packet->size);
#endif

#if OUTPUT_PCM
                fwrite(out_buffer, 1, out_buffer_size, pFile);//Write PCM
#endif
                index++;
            }

#if USE_SDL
            while(audio_len>0)//Wait until finish
            {
                SDL_Delay(1);
            }

            audio_chunk = (Uint8 *) out_buffer;//Set audio buffer (PCM data)
            audio_len =out_buffer_size;//Audio buffer length
            audio_pos = audio_chunk;
            SDL_PauseAudio(0);//Play
#endif
        }
        av_free_packet(packet);
    }

    swr_free(&au_convert_ctx);

#if USE_SDL
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
#endif

#if OUTPUT_PCM
    fclose(pFile);// Close file
#endif
    av_free(out_buffer);
    avcodec_close(pCodecCtx);// Close the codec
    avformat_close_input(&pFormatCtx); // Close the video file
}

