#include <string>
#include "flutter/lib/ui/media/encoder.h"


namespace flutter {

#define FFMPEG_TIME_BASE {1, 1000000000}

AVFrame* MakeVideoFrame(int w, int h, int format) {
    AVFrame* f = av_frame_alloc();
    f->width = w;
    f->height = h;
    f->format = format;  // AV_PIX_FMT_YUV420P / AV_PIX_FMT_RGBA
    av_frame_get_buffer(f, 32);
    return f;
}

void PacketRescaleTs(AVPacket* pkt, AVRational tbSrc, AVRational tbDst) {
  pkt->dts = av_rescale_q(pkt->dts, tbSrc, tbDst);
  pkt->pts = av_rescale_q(pkt->pts, tbSrc, tbDst);
}
  
MediaEncoder::MediaEncoder() {
  videoIndex_ = -1;
  audioIndex_ = -1;
  isWrittenHeader_ = false;
  packet_ = av_packet_alloc();
  combine_ = false;
  
  ////////////////////////////////////
  // 初始化编码器
  AVCodec *codec = avcodec_find_encoder_by_name("libx264");
  if (codec == nullptr) {
      printf("avcodec_find_encoder_by_name failed");
      codec = avcodec_find_encoder(AV_CODEC_ID_H264);
      if (codec == nullptr) {
        printf("avcodec_find_encoder(AV_CODEC_ID_H264) failed");
        return;
      }
  }
  videoContext_ = avcodec_alloc_context3(codec);
  videoContext_->profile = FF_PROFILE_H264_MAIN;
  videoContext_->pix_fmt = AV_PIX_FMT_YUV420P;
  videoContext_->width = width_;
  videoContext_->height = height_;

  videoContext_->framerate = (AVRational) {fps_, 1};
  videoContext_->time_base = (AVRational) {1, fps_};
  videoContext_->max_b_frames = 0;
  videoContext_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

  const char* preset = "superfast";
  av_opt_set(videoContext_->priv_data, "preset", preset, 0);
  av_opt_set(videoContext_->priv_data, "tune", "zerolatency", 0);
  av_opt_set(videoContext_->priv_data, "threads", "auto", 0);

  // 设置码率限制
  // if (bps != 0) {
  //     codecContext_->bit_rate = bps;
  //     codecContext_->rc_max_rate = bps;
  //     codecContext_->rc_buffer_size = bps >> 2;
  // }

  if (avcodec_open2(videoContext_, codec, nullptr) < 0) {
      printf("avcodec_open2 failed\n");
      return;
  }
}

MediaEncoder::~MediaEncoder() {
};

void MediaEncoder::Open() {
  // 初始化写文件句柄
  std::string url = "fluttervideo.mp4";
  int ret = avformat_alloc_output_context2(&fmtContext_, nullptr, nullptr, url.c_str());
  if (ret < 0) {
      printf("avformat_alloc_output_context2 failed");
      return;
  }
  
  AVStream *stream = avformat_new_stream(fmtContext_, nullptr);
  stream->codecpar->codec_tag = 0;
  ret = avcodec_parameters_from_context(stream->codecpar, videoContext_);
  if (ret < 0) {
      printf("avcodec_parameters_from_context failed\n");
      return;
  }
  videoIndex_ = stream->index;

  if (faststart_) {
      av_opt_set(fmtContext_->priv_data, "movflags", "faststart", 0);
  }

  videoTimeBase_ = (AVRational) {1, 12800};
  stream->time_base = videoTimeBase_;


  ret = avio_open(&fmtContext_->pb, url.c_str(), AVIO_FLAG_READ_WRITE);
  if (ret < 0) {
      printf("avio_open failed\n");
      return;
  }

  ret = avformat_write_header(fmtContext_, nullptr);
  if (ret < 0) {
      printf("avformat_write_header failed\n");
      return;
  }
  isWrittenHeader_ = true;
};

void MediaEncoder::Close() {
  WriteFrame(nullptr);
  if (videoContext_ != nullptr) {
      avcodec_free_context(&videoContext_);
      videoContext_ = nullptr;
  }

  if (fmtContext_ != nullptr) {
      if (isWrittenHeader_) {
          av_write_trailer(fmtContext_);
      }
      if ((fmtContext_ != nullptr) && (0 == (fmtContext_->flags & AVFMT_NOFILE))) {
          avio_closep(&fmtContext_->pb);
      }

      avformat_free_context(fmtContext_);
      fmtContext_ = nullptr;
  }

  if (packet_) {
      av_packet_free(&packet_);
      packet_ = nullptr;
  }

};

void MediaEncoder::WriteFrame(AVFrame* frame) {
  printf("write frame....\n");
  if (videoContext_ == nullptr) {
    return;
  }

  int ret = avcodec_send_frame(videoContext_, frame);
  if (ret < 0) {
      printf("video avcodec_send_frame failed\n");
      av_frame_free(&frame);
      return;
  }
  av_frame_free(&frame);
  while (ret >= 0) {
    ret = avcodec_receive_packet(videoContext_, packet_);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return;
    } else if (ret < 0) {
        printf("video avcodec_receive_packet failed\n");
        return;
    }
    WritePacket(packet_);
    av_packet_unref(packet_);
  }
};

void MediaEncoder::WritePacket(AVPacket *pkt) {
  PacketRescaleTs(pkt, (AVRational) FFMPEG_TIME_BASE, videoTimeBase_);
  pkt->stream_index = videoIndex_;
  int ret = av_interleaved_write_frame(fmtContext_, pkt);
  if (ret < 0) {
      printf("video av_interleaved_write_frame failed\n");
  }
  if (combine_) {
      av_packet_free(&pkt);
  }
}

void MediaEncoder::WriteImage(SkBitmap bitmap) {
  int w = bitmap.height();
  int h = bitmap.width();
  SwsContext* ctx = sws_getContext(w, h, AV_PIX_FMT_RGBA,
            width_, height_, AV_PIX_FMT_YUV420P,
            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
  AVFrame* frame = MakeVideoFrame(w, h, AV_PIX_FMT_RGBA);
  AVFrame* out = MakeVideoFrame(width_, height_, AV_PIX_FMT_YUV420P);
  memcpy(frame->data[0], bitmap.getPixels(), bitmap.rowBytes() * height_);
  sws_scale(ctx, &frame->data[0], &frame->linesize[0], 0, frame->height, &out->data[0], &out->linesize[0]);
  av_frame_free(&frame);
  sws_freeContext(ctx);
  WriteFrame(out);
};

}


