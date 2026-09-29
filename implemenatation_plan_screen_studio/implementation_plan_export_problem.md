# Goal Description
Resolve the 0kb export issue on macOS by transitioning the `ExportEngine` from a real-time UI-bound `QVideoSink` approach to a true offline background export using `AVAssetImageGenerator`.

The previous approach relied on capturing `QVideoFrame` objects from the UI's active `QMediaPlayer`. However, because Qt 6 hardware-accelerated video frames (Metal) cannot easily be mapped to CPU memory (`QImage`) without a valid Rendering Hardware Interface (RHI) context active on the calling thread, `frame.toImage()` consistently returns `NULL`. This results in no frames being appended to the `AVAssetWriter`, producing a 0kb output file.

## Open Questions
None. This is an internal architectural fix to bypass Qt 6 Metal texture cache limitations.

## Proposed Changes

### Export Backend (macOS)
#### [NEW] `src/export/interfaces/IMediaDecoder.h`
Create an interface for offline video frame extraction.
- `bool initialize(const std::string& videoPath)`
- `QImage getFrameAtTime(qint64 timeMs)`

#### [MODIFY] `src/export/macos/MacMediaEncoder.mm`
Implement `MacMediaDecoder` using `AVAssetImageGenerator` to extract frames natively via AVFoundation.
- Use `copyCGImageAtTime` to retrieve precise frames offline.
- Convert `CGImageRef` to `QImage` using CoreGraphics (`CGBitmapContextCreate`).
- Add a factory function `createMediaDecoder()`.

### Export Engine
#### [MODIFY] `src/export/ExportEngine.h`
- Remove dependency on UI `QVideoSink`.
- Add an `IMediaDecoder` instance.
- Add a background thread/QThread for the export loop so the UI remains responsive during offline rendering.

#### [MODIFY] `src/export/ExportEngine.cpp`
- In `startExport`, initialize the `IMediaDecoder` with the source video path.
- Spin up a background thread that iterates from `0` to `totalDuration` in steps of `1000/60` (60fps).
- Inside the loop:
  - Extract the exact frame via `IMediaDecoder::getFrameAtTime`.
  - Evaluate the `PlaybackEngine` camera/cursor states for that specific `timeMs` using its pure math functions (thread-safe since they just evaluate keyframes).
  - Paint the transforms and cursor onto the `QImage`.
  - Append to `IMediaEncoder`.
  - Emit `progressChanged`.
- Eliminate the hacky `QVideoSink` listener and monotonic PTS enforcement since the offline loop guarantees strictly sequential PTS.

## Verification Plan
### Automated Tests
N/A

### Manual Verification
Click the "Export" button in the UI. Ensure a valid MP4 is created with the exact zoom and cursor effects, and check that the file size is > 0kb. The UI should remain responsive while exporting.
