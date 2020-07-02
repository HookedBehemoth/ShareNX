#include "video_decoder.hpp"

#include "../translation/translation.hpp"

#include <album.hpp>
#include <borealis.hpp>

#define AV_THROW(av_expr)                    \
    ({                                       \
        int tmp_res = (av_expr);             \
        if (tmp_res < 0) {                   \
            PrintAVError(#av_expr, tmp_res); \
            throw OPEN_FAILED;               \
        }                                    \
    })

namespace album {

    namespace {

        void PrintAVError(const char *function, int ret) {
            char errno_buffer[AV_ERROR_MAX_STRING_SIZE];
            brls::Logger::info("%s: %s", function, av_make_error_string(errno_buffer, 0x30, ret));
        }

        struct ScopeGuard {
            std::function<void()> f;
            ScopeGuard(std::function<void()> f) : f(f) {}
            ~ScopeGuard() {
                if (f)
                    f();
            }
        };

    }

    VideoDecoder::VideoDecoder(const CapsAlbumFileId &fileId) : fileId(fileId) {
        ScopeGuard guard([this]() { this->~VideoDecoder(); });

        R_THROW_UNLESS(MovieReader::Start(fileId), OPEN_FAILED);
        MovieReader::seek(nullptr, 0, SEEK_SET);

        auto buffer = static_cast<unsigned char *>(av_malloc(IoBufferSize));

        this->ioCtx     = avformat_alloc_context();
        this->ioCtx->pb = avio_alloc_context(buffer, IoBufferSize, 0, nullptr, &MovieReader::read_packet, nullptr, &MovieReader::seek);

        AV_THROW(avformat_open_input(&this->ioCtx, "dummyFilename", nullptr, nullptr));
        AV_THROW(avformat_find_stream_info(this->ioCtx, nullptr));

        this->OpenContext(this->videoCtx, AVMEDIA_TYPE_VIDEO, 4);
        //this->OpenContext(this->audioCtx, AVMEDIA_TYPE_AUDIO, 1);

        this->frame = av_frame_alloc();
        if (this->frame == nullptr) {
            brls::Logger::error("Could not allocate frame");
            throw OUT_OF_MEMORY;
        }

        av_init_packet(&package);
        package.data = NULL;
        package.size = 0;

        guard.f = nullptr;
    }

    VideoDecoder::~VideoDecoder() {
        if (frame != nullptr)
            av_frame_free(&frame);

        //if (audioCtx.ctx != nullptr)
        //    avcodec_free_context(&audioCtx.ctx);

        if (videoCtx.ctx != nullptr)
            avcodec_free_context(&videoCtx.ctx);

        if (ioCtx != nullptr) {
            if (ioCtx->pb && ioCtx->pb->buffer)
                av_free(ioCtx->pb->buffer);
            avformat_close_input(&ioCtx);
        }

        MovieReader::Close();
    }

    /* I think I would want this to decoded on a dedicated thread and hold an audio and an image frame ready. */
    /* In testing I had to receive 30 frames before I received audio data. */
    /* Not sure if I missunderstand how ffmpeg works or if this is the only way. */
    AVFrame *VideoDecoder::ReceiveFrame() {
        /* free frame */
        av_frame_unref(frame);

        int ret = 0;
        while (true) {
            /* Try receive the frame */
            ret = avcodec_receive_frame(videoCtx.ctx, frame);
            if (ret == 0)
                return frame;

            /**/
            if (ret != AVERROR_EOF && ret != AVERROR(EAGAIN)) {
                PrintAVError("avcodec_receive_frame", ret);
                break;
            }

            /**/
            if (av_read_frame(ioCtx, &package) < 0) {
                brls::Logger::info("reached end");
                av_seek_frame(ioCtx, videoCtx.index, 0, AVSEEK_FLAG_ANY);
                this->videoCtx.ctx->frame_number = 0;
                continue;
            }

            /**/
            if (package.stream_index != videoCtx.index)
                continue;

            /* Send packet */
            ret = avcodec_send_packet(videoCtx.ctx, &package);

            /* free packet */
            av_packet_unref(&package);

            if (ret < 0) {
                PrintAVError("avcodec_send_packet", ret);
                if (ret == AVERROR_INVALIDDATA) {
                    album::MovieReader::seek(nullptr, 0, SEEK_SET);
                    continue;
                }
                break;
            }
        }

        return nullptr;
    }

    int VideoDecoder::GetProgress() {
        return this->videoCtx.ctx ? this->videoCtx.ctx->frame_number : 0;
    }

    void VideoDecoder::OpenContext(DecoderContext &context, enum AVMediaType type, int threadCount) {
        /* Find stream */
        int res = av_find_best_stream(this->ioCtx, type, -1, -1, nullptr, 0);
        AV_THROW(res);

        context.index  = res;
        context.stream = ioCtx->streams[context.index];

        /* find decoder for the stream */
        auto dec = avcodec_find_decoder(context.stream->codecpar->codec_id);
        if (dec == nullptr)
            throw OPEN_FAILED;

        /* Allocate a codec context for the decoder */
        context.ctx = avcodec_alloc_context3(dec);
        if (context.ctx == nullptr)
            throw OUT_OF_MEMORY;

        /* Allow for multithreading */
        if (threadCount > 0) {
            context.ctx->thread_count = threadCount;
            context.ctx->thread_type  = FF_THREAD_FRAME;
        }

        /* Copy codec parameters from input stream to output codec context */
        AV_THROW(avcodec_parameters_to_context(context.ctx, context.stream->codecpar));

        /* Init the decoders */
        AVDictionary *opts = nullptr;
        AV_THROW(avcodec_open2(context.ctx, dec, &opts));
    }

}
