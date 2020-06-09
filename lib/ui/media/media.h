// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_MediaRecorder_H_
#define FLUTTER_LIB_UI_MediaRecorder_H_

#include <map>
#include <mutex>
#include <string>

#include "flutter/fml/macros.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace flutter {

class Scene;

class MediaRecorder {
 public:
  MediaRecorder();

  ~MediaRecorder();

  void Start();
  void Stop();
  void Render(Scene* scene);


  static MediaRecorder *instance;
  static void RegisterNatives(tonic::DartLibraryNatives* natives);
 private:

  mutable std::mutex mutex_;


  FML_DISALLOW_COPY_AND_ASSIGN(MediaRecorder);
};


}  // namespace flutter

#endif  // FLUTTER_LIB_UI_MediaRecorder_H_
