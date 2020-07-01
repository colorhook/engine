// @dart = 2.6
part of dart.ui;


@pragma('vm:entry-point')
class MediaRecorder extends NativeFieldWrapperClass2 {

  VoidCallback onFrameFinished;

  @pragma('vm:entry-point')
  MediaRecorder() { 
    print("Dart create MediaRecorder");
    _constructor(() {
      if (this.onFrameFinished != null) {
        this.onFrameFinished();
      }
    });
  }

  start() {
    print("Dart start MediaRecorder");
    _start();
  }

  stop() {
    print("Dart stop MediaRecorder");
    _stop();
  }

  render(Scene scene) {
    print("Dart render MediaRecorder");
    _render(scene);
  }


  void _constructor(Function recordCallback) native 'MediaRecorder_constructor';
  void _render(Scene scene) native 'MediaRecorder_render';
  void _start() native 'MediaRecorder_start';
  void _stop() native 'MediaRecorder_start';
}