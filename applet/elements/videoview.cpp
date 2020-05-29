#include "videoview.hpp"

#include <util/caps.hpp>

namespace {

    void brls_av_error(const char *function, int ret) {
        char errno_buffer[0x30];
        av_strerror(ret, errno_buffer, 0x30);
        brls::Logger::info("%s: [%d] %s", function, ret, errno_buffer);
    }

    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type, int thread_count) {
        int ret, stream_index;
        AVStream *st;
        AVDictionary *opts = NULL;

        ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
        if (ret < 0) {
            brls::Logger::error("Could not find %s stream in input file '%s'",
                                av_get_media_type_string(type), "sample");
            return ret;
        } else {
            stream_index = ret;
            st           = fmt_ctx->streams[stream_index];

            /* find decoder for the stream */
            const AVCodec *dec = avcodec_find_decoder(st->codecpar->codec_id);
            if (!dec) {
                brls::Logger::error("Failed to find %s codec",
                                    av_get_media_type_string(type));
                return AVERROR(EINVAL);
            }

            /* Allocate a codec context for the decoder */
            *dec_ctx = avcodec_alloc_context3(dec);
            if (!*dec_ctx) {
                brls::Logger::error("Failed to allocate the %s codec context",
                                    av_get_media_type_string(type));
                return AVERROR(ENOMEM);
            }

            /* Allow for multithreading */
            if (thread_count) {
                (*dec_ctx)->thread_count = thread_count;
                (*dec_ctx)->thread_type  = FF_THREAD_FRAME;
            }

            /* Copy codec parameters from input stream to output codec context */
            if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
                brls::Logger::error("Failed to copy %s codec parameters to decoder context",
                                    av_get_media_type_string(type));
                return ret;
            }

            /* Init the decoders */
            if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
                brls::Logger::error("Failed to open %s codec",
                                    av_get_media_type_string(type));
                return ret;
            }
            *stream_idx = stream_index;
        }

        return 0;
    }

}

MovieView::MovieView(const CapsAlbumFileId &fileId) : AlbumView(fileId) {
    this->registerAction("Pause", brls::Key::Y, [this] {
        this->running = !this->running;
        this->updateActionHint(brls::Key::Y, this->running ? "Pause" : "Play");
        return true;
    });

    if (R_FAILED(album::MovieReader::Start(fileId))) {
        brls::Logger::error("Failed to start read");
        return;
    }

    auto buffer = static_cast<unsigned char *>(av_malloc(0x2000));

    fmt_ctx     = avformat_alloc_context();
    fmt_ctx->pb = avio_alloc_context(buffer, 0x2000, 0, nullptr, &album::MovieReader::read_packet, nullptr, &album::MovieReader::seek);

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, "dummyFilename", nullptr, nullptr) < 0) {
        brls::Logger::error("Could not open remote file");
        return;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        brls::Logger::error("Could not find stream information");
        return;
    }

    /* Open video context */
    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO, 4) < 0) {
        brls::Logger::error("Failed to open video context");
        return;
    }

    video_stream = fmt_ctx->streams[video_stream_idx];

    /* allocate image where the decoded image will be put */
    width   = video_dec_ctx->width;
    height  = video_dec_ctx->height;
    pix_fmt = video_dec_ctx->pix_fmt;

    int framerate = video_dec_ctx->framerate.num / video_dec_ctx->framerate.den;
    if (framerate == 0)
        framerate = 30;
    brls::Application::setMaximumFPS(framerate);

    frame = av_frame_alloc();
    if (!frame) {
        brls::Logger::error("Could not allocate frame");
        return;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    try {
        decodeWorkBuffer = new u8[width * height * 4];
    } catch (std::exception &e) {
        brls::Logger::error("Failed to allocate memory with: %s", e.what());
        return;
    }

    this->image.setRGBAImage(width, height, decodeWorkBuffer);

    running = true;
}

MovieView::~MovieView() {
    running = false;
    if (decodeWorkBuffer)
        delete[] decodeWorkBuffer;
    av_frame_free(&frame);
    avcodec_free_context(&video_dec_ctx);
    av_free(fmt_ctx->pb->buffer);
    avformat_close_input(&fmt_ctx);
    album::MovieReader::Close();
    brls::Application::setMaximumFPS(60);
}

void MovieView::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
    while (running && (tryReceive(video_dec_ctx) != 0)) {
        if (av_read_frame(fmt_ctx, &pkt) >= 0) {
            int ret = 0;

            if (pkt.stream_index != video_stream_idx)
                continue;

            /* Send packet */
            ret = avcodec_send_packet(video_dec_ctx, &pkt);

            /* free packet */
            av_packet_unref(&pkt);

            if (ret < 0) {
                brls_av_error("avcodec_send_packet", ret);
                break;
            }
        } else {
            brls::Logger::info("reached end");
            av_seek_frame(fmt_ctx, video_stream_idx, 0, AVSEEK_FLAG_ANY);
        }
    }

    AlbumView::draw(vg, x, y, width, height, style, ctx);
}

bool MovieView::tryReceive(AVCodecContext *dec) {
    int ret = avcodec_receive_frame(dec, frame);
    if (ret == 0) {
        /* Output */
        if (frame->width != width || frame->height != height || frame->format != pix_fmt) {
            brls::Logger::error("format changed: [w: %d, h: %d, f: %s] -> [w: %d, h: %d, f: %s]",
                                width, height, av_get_pix_fmt_name(pix_fmt),
                                frame->width, frame->height, av_get_pix_fmt_name((AVPixelFormat)frame->format));
        } else {
            /* update image */
            this->image.updateYUV((unsigned char **)frame->data, frame->linesize, decodeWorkBuffer);
        }

        /* free frame */
        av_frame_unref(frame);
    }
    if (ret != 0 && ret != AVERROR_EOF && ret != AVERROR(EAGAIN))
        brls_av_error("avcodec_receive_frame", ret);

    return ret;
}
