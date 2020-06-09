#include "flutter/lib/ui/media/media.h"

#include "flutter/lib/ui/compositing/scene.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/dart_microtask_queue.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {



void MediaRecorderStart(Dart_NativeArguments args) {
  
}

void MediaRecorderStop(Dart_NativeArguments args) {
  
}

void MediaRecorderRender(Dart_NativeArguments args) {
  // Dart_Handle exception = nullptr;
  // Scene* scene =
  //     tonic::DartConverter<Scene*>::FromArguments(args, 1, exception);
  // if (exception) {
  //   Dart_ThrowException(exception);
  //   return;
  // }
}



MediaRecorder::MediaRecorder() {

}

MediaRecorder::~MediaRecorder() {

}


void MediaRecorder::Start() {
  printf("MediaRecorder::Start\n");
}

void MediaRecorder::Render(Scene* scene) {
  printf("MediaRecorder::Render\n");
}

void MediaRecorder::Stop() {
  printf("MediaRecorder::Stop\n");
}


void MediaRecorder::RegisterNatives(tonic::DartLibraryNatives* natives) {
  // instance = new MediaRecorder();
  natives->Register({
      {"MediaRecorderStart", MediaRecorderStart, 1, true},
      {"MediaRecorderRender", MediaRecorderRender, 2, true},
      {"MediaRecorderStop", MediaRecorderStop, 1, true},
  });
}

}  // namespace flutter
