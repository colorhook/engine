
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/media/media.h"
#include "flutter/lib/ui/compositing/scene.h"

#include "flutter/fml/make_copyable.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"

#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_persistent_value.h"
#include "third_party/tonic/logging/dart_invoke.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"
#include "third_party/tonic/typed_data/typed_list.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, MediaRecorder);

static void MediaRecorder_constructor(Dart_NativeArguments args) {
  UIDartState::ThrowIfUIOperationsProhibited();
  DartCallConstructor(&MediaRecorder::Create, args);
}

#define FOR_EACH_BINDING(V) \
  V(MediaRecorder, start) \
  V(MediaRecorder, stop) \
  V(MediaRecorder, render)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

fml::RefPtr<MediaRecorder> MediaRecorder::Create(Dart_Handle callback) {
  return fml::MakeRefCounted<MediaRecorder>(callback);
}

MediaRecorder::MediaRecorder(Dart_Handle callback) {
  printf("engine MediaRecorder created\n");

  thread_ = std::make_unique<fml::Thread>("MediaRecorder");
  encoder_ = new MediaEncoder();
  auto dart_state = UIDartState::Current();
  
  if (!Dart_IsNull(callback)) {
    callback_.Set(dart_state, callback);
  }
}


MediaRecorder::~MediaRecorder() {
};

/**
 * 执行 Dart 中的 Function Closure
 */ 
void InvokeDartClosure(const tonic::DartPersistentValue& closure) {
  auto dart_state = closure.dart_state().lock();
  if (!dart_state) {
    return;
  }

  tonic::DartState::Scope scope(dart_state);
  auto dart_handle = closure.value();

  FML_DCHECK(dart_handle && !Dart_IsNull(dart_handle) &&
             Dart_IsClosure(dart_handle));
  UIDartState::Current()->ScheduleMicrotask(dart_handle);
}

/**
 * 调用 Dart 中的下一帧渲染逻辑 
 */ 
void MediaRecorder::RequestFrame() {
  InvokeDartClosure(callback_);
}

/**
 * Dart 中的帧渲染逻辑完成后，Dart 会调用该方法，执行视频帧编码
 */  
void MediaRecorder::render(Dart_Handle handle) {
  printf("engine MediaRecorder render scene\n");
  if (!Dart_IsNull(handle)) {
    Scene* scene =
      tonic::DartConverter<Scene*>::FromDart(handle);

    auto layer_tree_ = scene->takeLayerTree();
    auto size = layer_tree_->frame_size();
    if (!layer_tree_) {
      printf("Scene did not contain a layer tree.\n");
      return; 
    }

    auto picture = layer_tree_->Flatten(SkRect::MakeWH(size.width(), size.height()));
    if (!picture) {
      printf("Could not flatten scene into a layer tree.\n");
      return;
    }

    // Kick things off on the GPU.
    auto* dart_state = UIDartState::Current();
    auto gpu_task_runner = dart_state->GetTaskRunners().GetRasterTaskRunner();
    auto snapshot_delegate = dart_state->GetSnapshotDelegate();
    auto ui_task_runner = dart_state->GetTaskRunners().GetUITaskRunner();

    auto runner_ = thread_->GetTaskRunner();

    fml::TaskRunner::RunNowOrPostTask(
      runner_,
      [ui_task_runner, snapshot_delegate, picture, size, this] {
        sk_sp<SkImage> result;
        SkImageInfo image_info = SkImageInfo::MakeN32Premul(
            size.width(), size.height(), SkColorSpace::MakeSRGB());

        sk_sp<SkSurface> surface = SkSurface::MakeRaster(image_info);

        surface->getCanvas()->drawPicture(picture);
        surface->getCanvas()->flush();

        sk_sp<SkImage> device_snapshot = surface->makeImageSnapshot();
        auto raster_image = device_snapshot->makeRasterImage();

        if (!raster_image) {
          FML_LOG(ERROR) << "Screenshot: unable to make raster image";
          return;
        }

        SkBitmap bitmap;
        if (!raster_image.get()->asLegacyBitmap(&bitmap)) {
            FML_LOG(ERROR) << "image data is invalid";
            return;
        }

        encoder_->WriteImage(bitmap);

      //   SkFILEWStream stream("image.png");
        
      //    // Copy it into a bitmap and return the same.
      //   SkPixmap pixmap;
      //   if (!cpu_snapshot->peekPixels(&pixmap)) {
      //     FML_LOG(ERROR) << "Screenshot: unable to obtain bitmap pixels";
      //     return;
      //   }
      //  // SkEncodeImage(&stream, pixmap, SkEncodedImageFormat::kPNG, 100);

        fml::TaskRunner::RunNowOrPostTask(
            ui_task_runner,
            [this]() { 
            this->RequestFrame();
        });
      });

    printf("scene = %p \n", scene);
  } else {
    printf("null scene\n");
  }

}

/**
 * 开始录制视频，基于 ffmpeg 打开 IO 流，创建视频容器，写入视频流头信息
 */ 
void MediaRecorder::start() {
  printf("engine MediaRecorder start\n");
  encoder_->Open();
  RequestFrame();
}

/**
 * 停止录制视频，关闭 IO 流
 */ 
void MediaRecorder::stop() {
  printf("engine MediaRecorder stop\n");
}

/**
 * 创建 Dart 绑定
 */ 
void MediaRecorder::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({
    {"MediaRecorder_constructor", MediaRecorder_constructor, 2, true},
    FOR_EACH_BINDING(DART_REGISTER_NATIVE)
  });
}

}  // namespace flutter
