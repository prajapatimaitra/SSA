# Audio Export & Editor UI Fixes

This plan outlines the fixes and additions for the three issues identified: missing audio in the final export, incorrect scaling of the video preview during fullscreen mode, and adding resolution options to the export UI.

## Open Questions
None! FFmpeg is installed on your system, so we can use it to natively and perfectly sync the audio without complex macOS AVFoundation rewrites.

## Proposed Changes

### 1. Export Engine (Audio Muxing)

We will modify the Export Engine to leverage FFmpeg via Qt's `QProcess` to mux the audio tracks precisely after the video is written.

#### [MODIFY] `src/export/ExportEngine.h`
- Add a `resolution` parameter to `startExport`.

#### [MODIFY] `src/export/ExportEngine.cpp`
- **Resolution override:** Update `startExport` to accept and apply the new `resolution` parameter (e.g., 720p, 1080p, 4K) directly.
- **Audio Muxing:** In `finishExport(bool success)`, if the export succeeded, we will spawn a synchronous `QProcess` that calls `ffmpeg`.
- It will read `system.m4a` and `mic.m4a`.
- It will apply `-ss` and `-to` flags based on `m_editorController->trimStart()` and `trimEnd()` to correctly trim the audio to match the video exactly.
- It will mix both audio tracks (if both exist) using `-filter_complex "[1:a][2:a]amix"`.
- It will embed the audio into the final `.mp4`.

### 2. Editor Backend (API Updates)

#### [MODIFY] `src/editor/EditorController.h`
- Update the signature: `Q_INVOKABLE void exportVideo(const QString& presetName, const QString& resolution);`

#### [MODIFY] `src/editor/EditorController.cpp`
- Update `exportVideo` to pass the `resolution` string through to `m_exportEngine->startExport`.

### 3. Editor User Interface (Fullscreen & Resolution Dropdown)

#### [MODIFY] `src/ui/EditorView.qml`
- **Fullscreen Fix:** Change `cameraContainer` width/height from `rawVideoWidth` to `previewArea.targetW` and `targetH`. Because `VideoOutput` uses `Stretch`, it will intelligently downscale the massive Retina video to the logical workspace bounds *before* the QML engine applies the `scaleFactor`. This mathematically guarantees the video will never overflow the aspect ratio box, even when maximizing the window to 4K displays!
- **Resolution UI:** Add a new `ComboBox` named `resolutionCombo` next to the Aspect Ratio combo box, containing `["Native", "720p", "1080p", "1440p", "4K"]`.
- Update the "Export" button's `onClicked` event to pass both `exportPresetCombo.currentText` and `resolutionCombo.currentText` to the C++ backend.

## Verification Plan

### Automated Tests
- `cd build && make -j8`

### Manual Verification
1. Launch the app and maximize the window; verify the video stays perfectly inside the crop box.
2. Record a video speaking into the microphone.
3. Open it in the editor, trim 2 seconds from the start, select `1080p` from the new Resolution dropdown, and click Export.
4. Verify the output video has audio that is perfectly synced and trimmed!
