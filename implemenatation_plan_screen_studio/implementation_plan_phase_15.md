# Phase 15 Implementation Plan: Webcam Integration

This phase integrates webcam recording alongside the screen recording, rendering it as a circular Picture-in-Picture (PiP) over the main video during both playback and final export.

## User Review Required

> [!IMPORTANT]
> Since we are using Qt 6, we can use `QMediaCaptureSession`, `QCamera`, and `QMediaRecorder` to smoothly handle webcam capture across all OS platforms (macOS, Windows, Linux).
> The webcam feed will be recorded into a separate `webcam.mp4` file inside the `.ssa` project bundle simultaneously alongside `video.mp4`. This ensures our non-destructive media pipeline remains intact.

## Proposed Changes

### 1. UI Layer (Recorder & Editor)
#### [MODIFY] `src/ui/RecorderView.qml`
- Add a ComboBox to detect and select available webcams using `QMediaDevices`.
- Pass the selected webcam ID down to the backend when starting the capture.

#### [MODIFY] `src/ui/EditorView.qml`
- Add a new circular `VideoOutput` layered on top of the main screen video preview.
- This will playback the `webcam.mp4` stream synchronously with the main `video.mp4` timeline.

### 2. Capture Engine
#### [NEW] `src/capture/WebcamRecorder.h` & `.cpp`
- Implement a dedicated controller utilizing `QCamera` and `QMediaRecorder` to save the webcam feed to `{projectDir}/webcam.mp4`.
- Integrate lifecycle management (start/stop) to match the main screen capture timing.

#### [MODIFY] `src/capture/CaptureController.cpp`
- Inject the `WebcamRecorder` and trigger it when `startCapture()` and `stopCapture()` are invoked.

### 3. Editor Playback Backend
#### [MODIFY] `src/editor/EditorController.h` & `.cpp`
- Instantiate a secondary `QMediaPlayer` specifically for loading `{projectDir}/webcam.mp4`.
- Bind its playback state (play/pause/seek) tightly to the main media player to ensure the webcam stays perfectly in sync with the screen video.
- Expose the secondary player to QML as a `Q_PROPERTY`.

### 4. Export Compositing (FFmpeg)
#### [MODIFY] `src/export/ExportEngine.h` & `.cpp`
- **Dual Decoding**: Extend `ExportEngine` to open and decode both `video.mp4` and `webcam.mp4` concurrently during the export loop.
- **CPU Compositing**: Before encoding the final output frame, we will take the current `webcam.mp4` frame, scale it, apply a circular alpha mask (and optional shadow), and composite it over the bottom-right corner of the `video.mp4` frame buffer.

## Verification Plan

### Manual Verification
1. Launch app, select a webcam from the dropdown, and record a 10-second clip.
2. In the Editor view, verify the webcam plays synchronously as a circular Picture-in-Picture over the screen recording.
3. Export the video and verify the final `.mp4` file also contains the composited webcam PIP.
