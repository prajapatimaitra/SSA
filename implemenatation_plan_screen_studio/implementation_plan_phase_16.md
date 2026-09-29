# Phase 16 Implementation Plan: AI & ML Integration

This phase integrates local Machine Learning processing for post-recording analysis, specifically focusing on generating audio transcriptions (subtitles) and setting the foundation for scene detection.

## Proposed Architecture

### 1. AI Engine Interface
#### [NEW] `src/ai/AIEngine.h` & `.cpp`
- A dedicated module running in a background thread that triggers automatically after `CaptureController` finishes recording.
- Defines an interface `IAIModel` with a `processAudio(audioFile)` and `processVideo(videoFile)` method.

### 2. Transcription (Subtitles)
#### [NEW] `src/ai/TranscriptionModel.h` & `.cpp`
- A model designed to take the recorded `audio.mp4` or `video.mp4` and run speech-to-text inference.
- Outputs standard WebVTT (`transcription.vtt`) or SRT files directly into the project bundle directory (e.g., `.ssa/transcription.vtt`).

### 3. Editor Subtitle UI
#### [MODIFY] `src/editor/EditorController.h` & `.cpp`
- Add a property `Q_PROPERTY(QString subtitleText)` that updates dynamically as the video plays, driven by parsing the `.vtt` file based on the current playback time.
#### [MODIFY] `src/ui/EditorView.qml`
- Add a new `Text` component overlaid on the `previewArea` that binds to `editorController.subtitleText`.
- Provide basic styling (black background with partial opacity, white text, centered at the bottom) mimicking professional subtitle styling.

## Open Questions & User Review Required

> [!WARNING]
> **Dependency and Compilation Time**
> Integrating actual ML libraries like **Whisper.cpp** (for speech-to-text) or **OpenCV/ONNX** (for computer vision scene detection) requires downloading large model weights (50MB - 500MB) and significantly modifying our `vcpkg` dependencies, which can take upwards of 20-30 minutes to compile locally on your machine.
> 
> **Question:** How would you like to proceed?
> 
> **Option A (Recommended for now):** Implement the full `AIEngine` architecture, UI subtitle overlay, and `.vtt` parser, but use a **Mock ML Model** that just outputs placeholder subtitles (e.g., "0:00 - 0:05: [Speaking placeholder]"). This proves the architecture works instantly.
(Option A: Build the entire architectural scaffolding, subtitle engine, .vtt generator, and QML subtitle overlay UI, but back it with a "mock" AI engine for now. This takes almost no time to compile and proves the architecture works immediately.)
> 
> **Option B:** Fully integrate `whisper.cpp` and OpenCV via CMake/vcpkg. Be prepared for a very long compilation step.

Please review this plan and let me know whether Option A or Option B works best for you!
**(option A)**