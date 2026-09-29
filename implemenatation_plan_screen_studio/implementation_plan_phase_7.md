# Implementation Plan: Phase 7 (Timeline & Playback Engine)

This phase focuses on building the **Playback Engine**, which is the absolute core of our non-destructive editing workflow. 

Since video encoding/decoding is a massive endeavor reserved for a later phase (Phase 10), we will build the Timeline Engine to perfectly interpolate and replay our saved **Mouse Metadata** over a blank canvas. This ensures our synchronization logic is rock solid before we introduce heavy video buffers.

## User Review Required

> [!IMPORTANT]
> To prove the engine works, we will simulate playback by reading the exact timestamps of your mouse clicks from the `.ssa` project bundle.
> When you press "Play" in the Editor, a simulated mouse cursor will move across the screen matching exactly what you did during the recording!
> 
> We will use a standard `QTimer` ticking at 60fps to drive the timeline in C++, and it will push the interpolated `(X, Y)` cursor position to QML on every frame. Is this approach approved?

## Proposed Changes

We will build the playback state machine and time-interpolation logic.

### Core Architecture

#### [NEW] [src/editor/PlaybackEngine.h](file:///Users/maitraprajapati/Desktop/ssA/src/editor/PlaybackEngine.h)
#### [NEW] [src/editor/PlaybackEngine.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/editor/PlaybackEngine.cpp)
A core service responsible for tracking the virtual "Playhead".
- Manages `isPlaying`, `currentTime`, and `duration`.
- Contains a 60fps tick timer.
- Holds the loaded `ProjectMetadata` to calculate exactly where the mouse was at any given microsecond.

#### [MODIFY] [src/editor/EditorController.h](file:///Users/maitraprajapati/Desktop/ssA/src/editor/EditorController.h)
#### [MODIFY] [src/editor/EditorController.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/editor/EditorController.cpp)
- Add a pointer to `PlaybackEngine`.
- Expose properties for QML: `currentPlaybackTime`, `totalDuration`, `cursorX`, `cursorY`, and `cursorState` (Up/Down).
- Add `Q_INVOKABLE` methods to `play()`, `pause()`, and `seek(double percent)`.

### Editor Interface

#### [MODIFY] [src/ui/EditorView.qml](file:///Users/maitraprajapati/Desktop/ssA/src/ui/EditorView.qml)
- Add Play/Pause buttons to the Timeline area.
- Bind the slider to `EditorController.currentPlaybackTime`.
- Create a visual representation of the mouse cursor (a simple red circle or cursor icon) overlaying the preview area, and bind its `x` and `y` positions directly to `EditorController.cursorX` and `cursorY`.

#### [MODIFY] [src/CMakeLists.txt](file:///Users/maitraprajapati/Desktop/ssA/src/CMakeLists.txt)
- Include the new `PlaybackEngine` files in the build.

## Verification Plan

### Manual Verification
- Start a new recording, move your mouse around slowly in a recognizable pattern (like drawing a circle), and click a few times.
- Stop the recording.
- Once inside the Editor, click **Play** in the new timeline.
- Verify that the simulated cursor recreates your exact movements perfectly in sync with the timeline slider!
