#pragma once

#include "albumview.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

class MovieView : public AlbumView {
  private:
    AVFormatContext *fmt_ctx      = NULL;
    AVCodecContext *video_dec_ctx = NULL;
    int width, height;
    enum AVPixelFormat pix_fmt;
    AVStream *video_stream = NULL;

    int video_stream_idx = -1;
    AVFrame *frame       = NULL;
    AVPacket pkt;

    u8 *decodeWorkBuffer = nullptr;

    int frameCount;
    bool running = false;

  public:
    MovieView(const CapsAlbumFileId &fileId, int frameCount);

    ~MovieView();

    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx);

    bool tryReceive(AVCodecContext *dec);
};
