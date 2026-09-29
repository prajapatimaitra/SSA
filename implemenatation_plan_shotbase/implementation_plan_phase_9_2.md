# Goal Description
Execute **Phase 9: Style, Recording, and Export Presets**. This phase expands the existing JSON template system into a robust, app-wide **Preset Engine**, enabling users to quickly switch between diverse workflows (e.g., "YouTube Export", "Developer Dark Theme", "Vertical Recording"). 

It combines UI, backend capture, and export logic to handle Custom Region Capture, Target FPS & Resolutions, Audio Source routing, and detailed video export configurations.

## User Review Required
> [!IMPORTANT]  
> **File Formats:** Per your approval, we will use `.m4a` (AAC codec) for the separate audio tracks to save massive disk space over raw `.wav`, and it is fully supported by our Qt backend.

> [!TIP]  
> **Preset Storage:** As defined in your scope, we will bypass SQLite for now and store these presets as JSON files in `~/.gemini/antigravity-ide/presets/`.

## Open Questions
- Does this combined plan look complete, or are there any other parameters you want included in the Recording or Export Configs?

## Proposed Changes

### 1. Preset Data Models
#### [MODIFY] `src/project/TemplateManager.h` & `TemplateManager.cpp`
- **Rename** the core concept from `Template` to `Preset`.
- A Preset will contain three nested structs:
  - `StyleConfig` (background color, cursor style, padding, aspect ratio).
  - `ExportConfig` (output format/codec, aspect ratio, export resolution [4K, 1440p, 1080p, 720p], quality [High/Med/Low], hw-accel toggle).
  - `RecordingConfig` (region capture rect, target FPS [30/60], capture resolution, audio source [Mic, System, Both]).
- Update serialization to read/write these to `~/.gemini/antigravity-ide/presets/` as `.json` files.

### 2. Recording Presets, Region Capture & Audio
### 1. Recording Configuration UI
#### [MODIFY] `src/ui/WorkspaceView.qml`
- Add dropdowns to the top header area (or a dedicated settings row) for:
  - **Capture Resolution:** Native, 4K, 1440p, 1080p, 720p
  - **Target FPS:** 30 FPS, 60 FPS
  - **Audio Source:** Microphone Only, System Audio Only, Both (Mic + System)
- Hook these options up to the `captureController.startCapture` call so they configure the recording dynamically.

### 2. Capture Engine Plumbing
#### [MODIFY] `src/capture/macos/MacScreenCapture.mm`
- Scale the `SCStreamConfiguration` width and height down to the requested Resolution (e.g. 1920x1080 for 1080p) instead of capturing native resolution if the user selects a lower quality.
- Ensure the FPS limits the stream output to the requested framerate.

### 3. Region Capture UI
#### [NEW] `src/ui/components/RegionSelector.qml`
- Create a transparent, `Qt::FramelessWindowHint` overlay.
- Handle mouse drag events to define a `QRect` and emit the crop area.
- Dim the unselected portions of the screen.

#### [MODIFY] `src/ui/WorkspaceView.qml`
- Instantiate `RegionSelector.qml`.
- When "Select Region" is clicked, open the overlay. Once the user selects a region, pass the `cropRect` to `CaptureController::startCapture`.

#### [MODIFY] `src/capture/CaptureController.cpp` & `MacScreenCapturer.mm`
- Plumb the `cropRect` parameter down to the ScreenCaptureKit session so it only records the selected region instead of the full screen.

## Verification Plan
1. Launch the app and view the Workspace.
2. Select "1080p", "60 FPS", and "Both (Mic + System)" from the new dropdowns.
3. Record a short clip and verify the resulting `video.mp4` is 1080p @ 60fps.
4. Test the "Select Region" tool on the Workspace, ensuring it dims the screen and allows a precise crop selection for recording.
