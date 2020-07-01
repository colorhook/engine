
#ifndef FLUTTER_LIB_UI_MediaEncoder_H_
#define FLUTTER_LIB_UI_MediaEncoder_H_

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include "third_party/skia/include/core/SkBitmap.h"

namespace flutter {


class MediaEncoder {
public:
  MediaEncoder();
  ~MediaEncoder();

  void Open();
  void Close();
  void WriteFrame(AVFrame* frame);
  void WriteImage(SkBitmap bitmap);
  void WritePacket(AVPacket* pkt);

private:
  int width_ = 1920;
  int height_ = 1080;

  AVCodecContext*  videoContext_;
  AVFormatContext* fmtContext_;
  AVPacket* packet_;
  int fps_ = 25;
  int videoIndex_;
  int audioIndex_;
  bool isWrittenHeader_ = false;
  bool combine_ = false;
  bool faststart_ = false;
  AVRational videoTimeBase_;
};

}  // namespace flutter


#endif // FLUTTER_LIB_UI_MediaEncoder_H_
