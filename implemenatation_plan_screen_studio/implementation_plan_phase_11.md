# Phase 11: Export (Architecture Revision)

We are so close! The hardware encoder is now flawlessly functioning without crashing and is generating `.mp4` files. 

However, as we predicted in the Open Questions earlier, the video inside the exported file is currently **blank**. This is because Qt's `MediaPlayer` relies heavily on asynchronous backend decoders, and when our `OffscreenRenderer` asks it to instantly seek and render a frame, the decoder simply hasn't finished loading the image yet, resulting in a black frame being sent to the encoder!

## The Solution: The `QVideoSink` Pipeline
To achieve 100% reliable, synchronized frame exports without having to write native decoders for Mac, Windows, and Linux, we are going to pivot slightly and use a much simpler and robust approach provided by Qt 6: **QVideoSink**.

By attaching a `QVideoSink` directly to a hidden `QMediaPlayer` in C++, we can let the player run naturally. Every time the player successfully decodes a frame, it emits a `videoFrameChanged` signal. We will catch that signal, draw our virtual cursor onto the frame using `QPainter`, and instantly pipe it into our newly-fixed Mac Hardware Encoder!

This approach completely eliminates the need for the fragile `OffscreenRenderer` and OpenGL `QQuickRenderTarget` (which caused the crashes), and guarantees that every single frame we encode actually contains the video image.

## Proposed Changes

### [MODIFY] `src/export/ExportEngine.h`
- Remove the `OffscreenRenderer` dependency.
- Add a hidden `QMediaPlayer` and `QVideoSink`.
- Add a `onVideoFrameChanged(const QVideoFrame &frame)` event listener.

### [MODIFY] `src/export/ExportEngine.cpp`
- **Setup**: Create a `QMediaPlayer` pointing to the recorded `.mp4` and attach the `QVideoSink`.
- **Start**: When you hit export, instead of a manual loop, we simply hit "play" on the hidden media player at 1.0x speed.
- **Process**: Every time `onVideoFrameChanged` fires:
  1. We convert the `QVideoFrame` to a `QImage`.
  2. We ask the `PlaybackEngine` where the virtual mouse cursor was at this exact timestamp.
  3. We use `QPainter` to draw the red cursor directly onto the image.
  4. We send the image to the Hardware Encoder.

### [MODIFY] `src/editor/EditorController.h` / `.cpp`
- Remove the `m_offscreenRenderer` initialization, cleaning up the codebase.

### [DELETE] `src/renderer/OffscreenRenderer.h / .cpp` & `src/ui/ExportScene.qml`
- These files are no longer necessary, drastically simplifying the architecture and removing all OpenGL context issues.

## User Review Required
> [!IMPORTANT]
> **Real-Time Exporting**
> 
> Because we are hooking into the media player's natural decoding loop, the export will process in **Real-Time**. This means a 10-second video will take exactly 10 seconds to export. This is very standard for basic video editors and ensures we don't drop any frames or hit cross-platform asynchronous decoding bugs.
> 
> Once approved, I will implement this robust pipeline!
