# Phase 9: Style, Recording, and Export Presets

## Goal Description
Implement **Phase 9**, which introduces quick-start configuration profiles (Presets) for Recording, Exporting, and Styling. This will allow users to quickly switch between different workflows, such as "YouTube Export" (16:9, 60fps), "Developer Dark Theme" (dark background, specific cursor), or "Vertical Recording" (9:16 aspect ratio). 

Since we already built a basic `TemplateManager` using JSON in Phase 8 for visual styles, Phase 9 will expand this system to handle comprehensive app-wide Presets.

## Finalized Scope

Based on your feedback, we are expanding the existing JSON template system to support app-wide Presets, avoiding SQLite entirely. 

### 1. Recording Presets & Region Capture
We will implement a custom area selection tool:
- A transparent, borderless overlay window (`RegionSelector.qml` or a C++ QWidget overlay) that dims the screen.
- Click-and-drag mechanics to draw a clear rectangle.
- Pass the resulting `QRect` to `CaptureController`, which will feed it into the platform's `IScreenCapturer` (e.g., ScreenCaptureKit's `sourceRect`).
- Recording Presets will store configurations like "Full Screen" vs "Custom Region".

### 2. Export Presets
The Export Preset will control the final render output with the following parameters:
- **Output Format & Codec:** MP4 (H.264), MP4 (HEVC/H.265), or GIF.
- **Aspect Ratio / Canvas Size:** Original (Native), Standard (16:9), Vertical (9:16), or Square (1:1).
- **Export Resolution:** 4K, 1440p, 1080p, 720p.
- **Quality (Bitrate/CRF):** High, Medium, Low.
- **Hardware Acceleration Toggle:** CPU vs GPU encoders.

### Target Framerate (FPS): 
Options for 30 FPS (standard) or 60 FPS (crucial for smooth cursor movements and high-end tutorials).
Target Capture Resolution: Since disk space is not a concern, offer a full range of resolutions for the raw capture (the app will scale the raw screen to this size while recording):

### Audio Source Selection: 
A single dropdown or choice selection with three options:
Microphone Only
System Audio Only (Desktop sounds)
Both (Mic + System Audio) - When selected, the application will record them simultaneously into two separate audio files (e.g., system.wav and mic.wav) within the project bundle. This ensures non-destructive editing, allowing the user to adjust the volume of their voice independently from the computer's background noise later in the editor.
 

## Proposed Changes

### 1. Preset Data Models
#### [MODIFY] `src/project/TemplateManager.h` & `TemplateManager.cpp`
- Rename the core concept from `Template` to `Preset`.
- A `Preset` will contain three sections: `StyleConfig` (existing), `ExportConfig` (resolution, aspect ratio, codec, quality, hw-accel), and `RecordingConfig` (region capture flags).
- Update serialization to read/write these to `~/.gemini/antigravity-ide/presets/` as `.json` files.

### 2. Region Selection UI & Integration
#### [NEW] `src/ui/components/RegionSelector.qml`
- A transparent overlay window (`Qt::FramelessWindowHint`, `Qt::WindowStaysOnTopHint`).
- Handles mouse press, position change, and release to draw a selection rectangle.
- Emits the selected `x, y, width, height`.

#### [MODIFY] `src/capture/interfaces/IScreenCapturer.h` & `src/capture/mac/MacScreenCapturer.mm`
- Add support for a `cropRect` or `sourceRect`. When configured, the capture stream will only capture this specific region of the screen.

### 3. Export Engine Integration
#### [MODIFY] `src/export/ExportEngine.h` & `ExportEngine.cpp`
- Update `startExport()` to take an `ExportConfig` object.
- Pass the aspect ratio and resolution down to the `IMediaEncoder` and the offline `CameraEngine`.
- Adjust FFmpeg flags based on the Quality (CRF) and Hardware Acceleration settings.

### 4. UI Integration
#### [MODIFY] `src/ui/EditorView.qml`
- Expand the Properties Panel to include the new Export Configuration dropdowns.
- Update the preview canvas aspect ratio immediately when the preset's aspect ratio changes.

#### [MODIFY] `src/ui/RecorderView.qml` / `WorkspaceView.qml`
- Add a "Select Region" button that triggers the `RegionSelector` before starting a recording.

### 3. UI Integration
#### [MODIFY] `src/ui/EditorView.qml`
- Expand the existing Templates dropdown into a more robust "Export Preset" selector that configures the `ExportEngine` before rendering.
- Ensure the selected preset's aspect ratio and visual styles are instantly reflected in the preview window.

#### [MODIFY] `src/ui/WorkspaceView.qml` / `RecorderView.qml`
- Add a UI dropdown before starting a recording to allow the user to select their Recording Preset (e.g., "Vertical Recording").

## Verification Plan
1. Launch the application and create a new Preset named "TikTok Format".
2. Set the preset's Style to a purple background, its Export to 1080x1920 (9:16) at 60fps, and save it.
3. Apply the preset to a project and verify the preview instantly updates to the purple background.
4. Trigger an export and verify the resulting MP4 matches the 1080x1920 resolution specified by the preset.
