#pragma once

#include <borealis.hpp>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

#include <chrono>

#ifndef __SWITCH__
#define VIDEO "/home/behemoth/Downloads/2020051915095700-16851BE00BC6068871FE49D98876D6C5.mp4";
#define THUMB "/home/behemoth/Downloads/562583696.bmp"
#else
#define VIDEO "/emuMMC/RAW1/Nintendo/Album/2020/05/20/2020052018194000-16851BE00BC6068871FE49D98876D6C5.mp4";
#define THUMB "/test/562583696.bmp"
#endif

namespace
{

static int open_codec_context(int* stream_idx, AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream* st;
    AVCodec* dec       = NULL;
    AVDictionary* opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        brls::Logger::error("Could not find %s stream in input file '%s'",
            av_get_media_type_string(type), "sample");
        return ret;
    }
    else
    {
        stream_index = ret;
        st           = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            brls::Logger::error("Failed to find %s codec",
                av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx)
        {
            brls::Logger::error("Failed to allocate the %s codec context",
                av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
        {
            brls::Logger::error("Failed to copy %s codec parameters to decoder context",
                av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
        {
            brls::Logger::error("Failed to open %s codec",
                av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}
}

class MovieView : public brls::View
{
  private:
    brls::Image image;
    AVFormatContext* fmt_ctx      = NULL;
    AVCodecContext* video_dec_ctx = NULL;
    int width, height;
    enum AVPixelFormat pix_fmt;
    AVStream* video_stream   = NULL;
    const char* src_filename = VIDEO;

    int video_dst_linesize[4];
    int video_dst_bufsize;

    int video_stream_idx = -1;
    AVFrame* frame       = NULL;
    AVPacket pkt;
    int video_frame_count = 0;

    uint8_t* rgba_buffer = NULL;

    bool running = false;

  public:
    MovieView()
        : View()
    {
        this->registerAction("Back", brls::Key::B, [] {brls::Application::popView(); return true; });
        this->image.setScaleType(brls::ImageScaleType::NO_RESIZE);
        this->image.setImage(THUMB);
        this->image.setParent(this);

        int ret = 0;

        /* open input file, and allocate format context */
        if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0)
        {
            brls::Logger::error("Could not open source file %s", src_filename);
            return;
        }

        /* retrieve stream information */
        if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
        {
            brls::Logger::error("Could not find stream information");
            return;
        }

        if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0)
        {
            video_stream = fmt_ctx->streams[video_stream_idx];

            /* allocate image where the decoded image will be put */
            width       = video_dec_ctx->width;
            height      = video_dec_ctx->height;
            rgba_buffer = new unsigned char[width * height * 4];

            if (ret < 0)
            {
                brls::Logger::error("Could not allocate raw video buffer");
                goto end;
            }
            video_dst_bufsize = ret;
        }

        /* dump input information to stderr */
        av_dump_format(fmt_ctx, 0, src_filename, 0);

        if (!video_stream)
        {
            brls::Logger::error("Could not find video stream in the input, aborting");
            ret = 1;
            goto end;
        }

        frame = av_frame_alloc();
        if (!frame)
        {
            brls::Logger::error("Could not allocate frame");
            ret = AVERROR(ENOMEM);
            goto end;
        }

        /* initialize packet, set data to NULL, let the demuxer fill it */
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;

        if (video_stream)
            brls::Logger::info("Demuxing video from file '%s'", src_filename);

        running = true;

        return;

    end:
        puts("Failed");
    }

    ~MovieView()
    {
        avcodec_free_context(&video_dec_ctx);
        avformat_close_input(&fmt_ctx);
        av_frame_free(&frame);
        free(rgba_buffer);
    }

    void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
    {
        this->image.setBoundaries(0, 0, 1280, 720);
        this->image.layout(vg, style, stash);
    }

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
    {
        if (running)
        {
        read:
            if (av_read_frame(fmt_ctx, &pkt) >= 0)
            {
                if (pkt.stream_index != video_stream_idx)
                    goto read;
                    
                if (decode_packet(video_dec_ctx, &pkt) < 0)
                {
                    decode_packet(video_dec_ctx, nullptr);
                    running = false;
                }
                av_packet_unref(&pkt);
            }
            else
            {
                running = false;
            }
        }

        this->image.draw(vg, 0, 0, 1280, 720, style, ctx);
    }

    brls::View* getDefaultFocus() final
    {
        return this;
    }

    int output_video_frame(AVFrame* frame)
    {
        if (frame->width != width || frame->height != height || frame->format != pix_fmt)
        {
            /* To handle this change, one could call av_image_alloc again and
         * decode the following frames into another rawvideo file. */
            brls::Logger::error("Error: Width, height and pixel format have to be "
                                "constant in a rawvideo file, but the width, height or "
                                "pixel format of the input video changed:\n"
                                "old: width = %d, height = %d, format = %s\n"
                                "new: width = %d, height = %d, format = %s",
                width, height, av_get_pix_fmt_name(pix_fmt),
                frame->width, frame->height,
                av_get_pix_fmt_name((AVPixelFormat)frame->format));
            return -1;
        }

        // update image
        this->image.updateYUV((unsigned char**)frame->data, frame->linesize, rgba_buffer);

        return 0;
    }

    int decode_packet(AVCodecContext* dec, const AVPacket* pkt)
    {
        int ret = 0;

        // submit the packet to the decoder
        ret = avcodec_send_packet(dec, pkt);
        if (ret < 0)
        {
            brls::Logger::error("Error submitting a packet for decoding (%d)", ret);
            return ret;
        }

        // get all the available frames from the decoder
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(dec, frame);
            if (ret < 0)
            {
                // those two return values are special and mean there is no output
                // frame available, but there were no errors during decoding
                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                    return 0;

                brls::Logger::error("Error during decoding (%d)", ret);
                return ret;
            }

            ret = output_video_frame(frame);

            av_frame_unref(frame);
            if (ret < 0)
                return ret;
        }

        return 0;
    }
};
