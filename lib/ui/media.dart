part of dart.ui;

void mediaRecorderStart() native 'MediaRecorderStart';
void mediaRecorderStop() native 'MediaRecorderStop';
void mediaRecorderRender(Scene scene) native 'MediaRecorderRender';

class MediaRecorder {
  @override
  String toString() {
    return 'MediaRecorder';
  }
}