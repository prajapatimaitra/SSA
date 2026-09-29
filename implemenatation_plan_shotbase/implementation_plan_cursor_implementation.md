# Smart Cursor Engine

We will utilize the actual cursor PNGs from the `cursors` folder and magically determine which one to use at any given moment during the recording (e.g. arrow, pointing hand, text I-beam)! 

## Open Questions
None. The research is complete and I have confirmed a way to do this using a fast, low-level heuristic without resorting to hacky OCR or AI on every frame!

## Proposed Changes

### 1. The Core Data Model
#### [MODIFY] `src/input/MouseEvent.h`
- Add `std::string cursorType` to the `MouseEvent` struct.

### 2. The Capture Engine Heuristic
#### [MODIFY] `src/capture/mac/MacScreenCapture.mm`
- In the background screen-capture loop (which runs at 60fps), we will poll the global `[NSCursor currentSystemCursor]`.
- **The Magic Hash Dictionary**: I wrote a script to extract the exact MD5 hash of the raw `TIFFRepresentation` for all standard macOS cursors. I will embed this dictionary (e.g., `c518be5b...` -> `"pointingHand"`, `6b6f08ad...` -> `"iBeam"`) directly into the C++ capture engine!
- For performance, we only hash the cursor when the memory address of the cursor object changes.
- When the cursor type changes, we will emit a new `MouseEvent` with the mapped string name (e.g., `"pointingHand"`) so it gets perfectly saved into the `metadata.json` timeline.

### 3. The Offline Render Engine
#### [MODIFY] `src/editor/PlaybackEngine.h` & `.cpp`
- Load the cursor PNGs from the `cursors` directory and cache them in memory as `QImage`.
- Update `evaluateCursorStateOffline` to return the `cursorType` `std::string` at any given timestamp.

#### [MODIFY] `src/export/ExportEngine.cpp`
- Instead of drawing a simple white dot/circle, the exporter will query the `cursorType` from the `PlaybackEngine`.
- It will fetch the corresponding high-res PNG (e.g., `pointingHand.png`).
- It will offset it by its unique hotspot center, scale it according to the user's `cursorScale` setting, and composite it beautifully over the motion-blurred video!

## Verification Plan
1. Recompile the app.
2. Record a quick 10-second video where we hover over a button (to get the hand) and a text field (to get the I-beam).
3. Export the video and verify that the gorgeous custom cursor PNGs dynamically switch states just like the real system cursor did!
