# Phase 13: Motion Blur (Temporal Supersampling)

## Goal Description
Implement cinematic motion blur for rapid camera pans and cursor movements. 

Because Screen Studio renders a virtual camera and cursor over a recorded video, the panning motion can look stuttery at standard framerates. By implementing a "shutter angle" simulation (Temporal Supersampling), we can blend multiple sub-frames together during offline export to create buttery-smooth motion blur on the camera and cursor, drastically increasing the professional quality of the output video.

## User Review Required
> [!IMPORTANT]
> **Performance Trade-off**: Temporal supersampling multiplies the rendering work during export. If we use 8 samples per frame (a standard for high-quality motion blur), the offline export will take roughly 8 times longer. 
> 
> Because of Qt 6 limitations, we cannot easily run this complex shader/blending logic in real-time in the QML preview. Therefore, **motion blur will only be visible in the final exported video**, not during live playback in the editor.

## Proposed Changes

### Export Engine
#### [MODIFY] `src/export/ExportEngine.cpp`
- **Sub-frame Iteration**: Instead of rendering exactly one frame at `currentTimeMs`, we will simulate a 180-degree or 360-degree shutter angle by rendering $N$ (e.g., 8) sub-frames spaced slightly apart around `currentTimeMs`.
- **Accumulation Buffer**: We will allocate a floating-point (or 16-bit integer) accumulation buffer for the image.
- **Blending**: For each sub-frame:
  1. Retrieve the exact mathematical state of the camera (X, Y, Zoom, Rotation) at `subTimeMs`.
  2. Retrieve the exact mathematical state of the cursor (X, Y) at `subTimeMs`.
  3. Draw the video frame and cursor into a temporary QImage.
  4. Add the temporary QImage's pixels to the accumulation buffer.
- **Final Output**: Divide the accumulation buffer by $N$ and convert it back to an 8-bit `QImage::Format_ARGB32` before passing it to the `MacMediaEncoder`.

### Engine Tuning
- The `PlaybackEngine`'s `evaluateAtTime` functions already support sub-millisecond precision because they use continuous Catmull-Rom spline interpolation. We just need to pass the sub-frame timestamps.

## Verification Plan
### Manual Verification
1. Open a recording with rapid mouse movements or a large spatial gap that causes a fast camera pan.
2. Export the video.
3. Pause the exported video during the pan—you should clearly see directional blurring on the cursor and the video content, confirming that motion blur is working.
