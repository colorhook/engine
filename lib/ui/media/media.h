#ifndef FLUTTER_LIB_UI_MediaRecorder_H_
#define FLUTTER_LIB_UI_MediaRecorder_H_

#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/fml/thread.h"
#include "flutter/fml/task_runner.h"
#include "flutter/lib/ui/media/encoder.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace flutter {

class Scene;

class MediaRecorder final : public RefCountedDartWrappable<MediaRecorder> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(MediaRecorder);

 public:
  static fml::RefPtr<MediaRecorder> Create(Dart_Handle handle);

  ~MediaRecorder() override;


  void start();
  void render(Dart_Handle handle);
  void stop();
  void RequestFrame();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);
 private:

  MediaRecorder(Dart_Handle callback);
  int fps_ = 25;


  std::unique_ptr<fml::Thread> thread_;
  tonic::DartPersistentValue callback_;
  MediaEncoder* encoder_;

  //FML_FRIEND_MAKE_REF_COUNTED(MediaRecorder);
  //FML_FRIEND_REF_COUNTED_THREAD_SAFE(MediaRecorder);
  FML_DISALLOW_COPY_AND_ASSIGN(MediaRecorder);
};


}  // namespace flutter

#endif  // FLUTTER_LIB_UI_MediaRecorder_H_
