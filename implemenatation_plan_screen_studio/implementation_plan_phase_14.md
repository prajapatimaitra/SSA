# Implementation Plan: Phase 14 (Advanced Editor)

Provide professional video editing capabilities within the application. This phase bridges the gap between raw recorded data and a polished final export by introducing UI components and backend logic for trimming, cursor styling, background customization, and camera keyframe manipulation.

## User Review Required
> [!IMPORTANT]
> **Data Persistence:** I plan to update the `project.json` (Phase 5) schema to save these new customizations (trim times, cursor colors, background properties). This ensures that if you close and reopen the app, your edits are saved. Does this align with your expectations?
> 
> **Timeline Trimming:** The trimming will be **non-destructive**. It will simply restrict the `PlaybackEngine` and `ExportEngine` to only play/export frames between `trimStartTime` and `trimEndTime`, leaving the raw `.mp4` untouched.

## Proposed Changes

### Core Data & Backend

#### [MODIFY] `src/project/ProjectMetadata.h`
- Add fields for visual state: `QString cursorColor`, `double cursorScale`, `QString backgroundColor`.
- Add fields for trimming: `uint64_t trimStartTimeMs`, `uint64_t trimEndTimeMs`.
- Ensure the JSON serializer loads and saves these new fields.

#### [MODIFY] `src/editor/EditorController.h` & `src/editor/EditorController.cpp`
- Expose the new metadata fields to QML via `Q_PROPERTY` (`cursorColor`, `cursorScale`, `backgroundColor`, `trimStart`, `trimEnd`).
- Enforce the `trimStart` and `trimEnd` limits during playback (e.g., automatically loop or stop when the playhead hits the `trimEnd`).
- Add `Q_INVOKABLE` functions to allow QML to create, move, or delete camera keyframes.

#### [MODIFY] `src/export/ExportEngine.cpp`
- Update the `runExportLoop` to start at `trimStart` and end at `trimEnd` instead of `0` to `totalDurationMs`.
- Update the composition logic to draw the background using `backgroundColor` instead of hardcoded black.
- Apply `cursorScale` and `cursorColor` when drawing the cursor overlay.

---

### QML Frontend

#### [MODIFY] `src/ui/EditorView.qml`
- **Right Sidebar (Properties Panel):** Add a collapsible right sidebar layout containing:
  - **Background settings:** Color picker or hex input.
  - **Cursor settings:** Color picker, scale slider.
- **Timeline Area Upgrades:**
  - Introduce a RangeSlider (or custom dual-handle control) overlaying the timeline to visually set the trim start and end points.
  - Draw visual diamond markers (`◆`) on the timeline track corresponding to existing Camera Keyframes. Make them horizontally draggable to update their timestamp via the `EditorController`.
- **Top Bar Export Presets:**
  - Next to the "Export" button, add a "Preset" ComboBox (e.g., "YouTube 4K", "TikTok 1080p", "Custom").
  - Wire this to automatically set the `aspectRatio` and scale the `videoWidth` / `videoHeight` appropriately before calling export.

## Verification Plan
### Automated & Manual Verification
- Launch the application, record a short clip, and open the editor.
- Modify the cursor color to Blue and scale to 2.0x, verifying the preview updates instantly.
- Drag the trim handles to cut the first and last 2 seconds.
- Export the video and manually verify using a video player (e.g., VLC or QuickTime) that the output respects the custom cursor, background, and precise trimmed length.
