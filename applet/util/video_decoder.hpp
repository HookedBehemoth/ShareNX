#pragma once

#include <switch.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace album {

    class VideoDecoder {

        struct DecoderContext {
            AVCodecContext *ctx = nullptr;
            AVStream *stream    = nullptr;
            int index           = -1;
        };

      private:
        static constexpr u32 IoBufferSize = 0x2000;
        const CapsAlbumFileId fileId;

        AVFormatContext *ioCtx = nullptr;
        AVFrame *frame         = nullptr;
        AVPacket package;

        DecoderContext videoCtx;
        //DecoderContext audioCtx;

      public:
        VideoDecoder(const CapsAlbumFileId &fileId);
        ~VideoDecoder();

        AVFrame *ReceiveFrame();
        //void ReceiveAudio();

        int GetProgress();

      private:
        void OpenContext(DecoderContext &context, enum AVMediaType type, int threadCount);
    };

}