# Phase 8: Manual Camera (Virtual Camera Engine)

Per the master blueprint, we are getting strictly back on track! We will now build the **Manual Camera Engine**.

The core philosophy of this app is that we record the raw 1080p/4k video, and we create dynamic zooms and pans *in software* using keyframes, completely non-destructively.

## Goal
Implement a `CameraEngine` that interpolates between `CameraKeyframes` over time, and expose this virtual camera state to QML so the video and cursor pan/zoom dynamically during playback.

## Proposed Changes

### C++ Core Data Structures
#### [NEW] [CameraKeyframe.h](file:///Users/maitraprajapati/Desktop/ssA/src/project/CameraKeyframe.h)
- Define a `CameraKeyframe` struct:
  - `qint64 timestampMs`
  - `double x` (Center X in project coordinates)
  - `double y` (Center Y in project coordinates)
  - `double zoom` (Scale factor, 1.0 = normal)
  - `double rotation` (Degrees)
  - `EasingType easing` (Enum for Linear, EaseInOut, Smoothstep)

#### [NEW] [CameraEngine.h](file:///Users/maitraprajapati/Desktop/ssA/src/playback/CameraEngine.h) & [CameraEngine.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/playback/CameraEngine.cpp)
- Implements the mathematical interpolation. Given a current timestamp in milliseconds and a list of `CameraKeyframe`s, it outputs the interpolated `x, y, zoom, rotation` for that exact frame.

### C++ Integration
#### [MODIFY] [PlaybackEngine.h](file:///Users/maitraprajapati/Desktop/ssA/src/playback/PlaybackEngine.h) & [PlaybackEngine.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/playback/PlaybackEngine.cpp)
- Add a `CameraEngine m_cameraEngine` member.
- Hook it into the `tick()` function to evaluate camera position alongside the cursor position.
- Emit a `cameraUpdated(double x, double y, double zoom, double rotation)` signal.

#### [MODIFY] [EditorController.h](file:///Users/maitraprajapati/Desktop/ssA/src/editor/EditorController.h) & [EditorController.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/editor/EditorController.cpp)
- Expose `cameraX`, `cameraY`, `cameraZoom`, and `cameraRotation` properties to QML.

### QML Rendering
#### [MODIFY] [EditorView.qml](file:///Users/maitraprajapati/Desktop/ssA/src/ui/EditorView.qml)
- Wrap the `VideoOutput` and `virtualCursor` inside a single transformation container `Item`.
- Bind the container's `scale` and `transform` properties to the `editorController.cameraZoom`, `cameraX`, and `cameraY`. This instantly applies the virtual camera view to the entire composition!

## Open Questions

> [!TIP]
> To test the `CameraEngine`, I will programmatically generate a couple of hardcoded keyframes when the project loads (e.g., zooming in to 2.0x at 2 seconds, and zooming back out at 5 seconds). We will build the UI to actually add/drag keyframes manually in Phase 14 (Advanced Editor).

Do you approve this plan to build the Virtual Camera layer?
