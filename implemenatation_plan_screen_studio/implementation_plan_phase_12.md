# Goal Description
Implement **Phase 12 (Advanced Camera)**, which introduces an Aspect Ratio Engine for vertical/square video export and Advanced Grouping for smoother camera pans during rapid clicks.

## Proposed Changes

### 1. Aspect Ratio Engine
#### [MODIFY] `src/editor/EditorController.h` & `EditorController.cpp`
- Add an `aspectRatio` property (e.g., `16:9`, `9:16`, `1:1`) to allow the user to switch the export format.
- Update `videoWidth()` and `videoHeight()` to dynamically return the padded/cropped resolution based on the selected aspect ratio (e.g., for a 1920x1080 recording, selecting `9:16` might output a `1080x1920` canvas).

#### [MODIFY] `src/playback/SmartZoomAnalyzer.h` & `SmartZoomAnalyzer.cpp`
- Modify `generateKeyframes` to accept the target aspect ratio canvas size.
- Update the camera clamping logic: when zoomed in, the camera's X and Y coordinates must be strictly clamped against the *new* aspect ratio bounds to prevent black bars from leaking into the frame during pans.

#### [MODIFY] `src/ui/EditorView.qml`
- Add a UI control (e.g., a ComboBox or Row of Buttons) to the top bar allowing the user to select the aspect ratio (`16:9`, `9:16`, `1:1`).
- Update the `cameraContainer` constraints to visually reflect the cropped canvas in the preview player in real-time.

### 2. Advanced Grouping Heuristics
#### [MODIFY] `src/playback/SmartZoomAnalyzer.cpp`
- Improve the sequence grouping algorithm.
- Instead of purely time-based sequence grouping, introduce a spatial threshold. If two rapid clicks are extremely far apart (e.g., > 60% of screen width), the camera will slightly "zoom out" during the pan (an arc trajectory) to reduce motion sickness, before zooming back in for the second click.
- For clicks that are close together, it will maintain the zoom level and execute a smooth, continuous pan as originally intended, eliminating jitter.

## Open Questions
- For the Aspect Ratio Engine, when exporting a 16:9 recording to 9:16, the video will be heavily cropped on the left and right. The Smart Zoom camera will automatically pan to follow the cursor within this narrow vertical slice. Does this align with your expectations for the vertical export feature?

## Verification Plan
1. Launch the editor and select `9:16 (Vertical)` in the UI.
2. Verify the preview updates to a vertical canvas and the camera automatically frames the cursor within those new bounds.
3. Verify rapid, spaced-out clicks result in an arc-pan (zooming out slightly during movement) rather than a direct linear pan.
