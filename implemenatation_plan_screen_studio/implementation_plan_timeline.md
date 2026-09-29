# Phase 14: Advanced Timeline UI

The user has requested the addition of advanced video progress bar features (similar to the interactive timeline found in Screen Studio). 

## Goal Description
We will replace the basic QML `Slider` at the bottom of the editor with a custom, high-quality, interactive Timeline component. This timeline will provide visual feedback for video duration, playhead position, and mouse click events, creating a much more professional editing experience.

## Open Questions
- Do you want to implement trimming (dragging the start and end of the timeline to shorten the video) in this phase, or should we just focus on the visual layout, playhead, and click markers for now?

## Proposed Changes

### UI Components
#### [NEW] `src/ui/components/TimelineRuler.qml`
- A visual ruler that displays time markings (e.g., 0:00, 0:10, 0:20) based on the total video duration.
- Handles snapping and dynamic label density based on the width of the window.

#### [NEW] `src/ui/components/TimelineTrack.qml`
- A rounded rectangle representing the video/action track.
- Will dynamically read the `EditorController`'s mouse click timestamps (via a new C++ method/property) to draw small visual markers (dots/ticks) on the track precisely where you clicked during the recording.

#### [NEW] `src/ui/components/Playhead.qml`
- A stylish vertical line with a scrubber handle at the top.
- Users can drag this handle to seek through the video smoothly.

#### [MODIFY] `src/ui/EditorView.qml`
- Remove the standard `Slider`.
- Assemble the `TimelineRuler`, `TimelineTrack`, and `Playhead` inside the bottom timeline area.
- Bind the playhead's X position to `editorController.currentTime` and `editorController.totalDuration`.

### Backend (C++)
#### [MODIFY] `src/editor/EditorController.h` / `.cpp`
- Add a new `Q_INVOKABLE` method or a `QVariantList` property that exposes the timestamps of all mouse clicks from the `ProjectMetadata`. This allows the QML `TimelineTrack` to iterate through them and draw the visual markers.

## Verification Plan
1. Launch the editor with a recording.
2. Verify that the new timeline is rendered at the bottom.
3. Verify that visual markers appear on the track corresponding to when the mouse was clicked.
4. Drag the playhead to ensure the video scrubs correctly and smoothly.
