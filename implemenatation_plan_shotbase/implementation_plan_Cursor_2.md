# Implementation Plan: Smart Cursor Engine & Zoom-In Mechanics

This implementation plan translates the theoretical mechanics, mathematical formulas, and visual algorithms detailed in [`CURSOR_AND_ZOOM_ANALYSIS.md`](file:///Users/maitraprajapati/Desktop/ssA31/implemenatation_plan_shotbase/CURSOR_AND_ZOOM_ANALYSIS.md) into concrete C++ / Qt6 code changes for the `ssA31` application.

## User Review Required

> [!IMPORTANT]
> **Analysis vs. Implementation Plan**: `CURSOR_AND_ZOOM_ANALYSIS.md` is a technical reference document analyzing cursor physics, anchor offsets, and zoom projection formulas. To implement this in `ssA31`, we will update `PlaybackEngine`, `ExportEngine`, and `MouseEvent` models.
> 
> **Key Enhancements Included**:
> 1. **Hotspot Anchor Alignment**: Anchor ratios $(anchorX, anchorY)$ for all 39 cursor types to guarantee zero tip displacement during cursor state swaps (e.g. Arrow $\leftrightarrow$ Hand $\leftrightarrow$ I-Beam).
> 2. **Sinusoidal Click Compression**: 8% cursor compression over a sine curve when a click occurs.
> 3. **Proportional Vector Click Animations**: Ripple, Spotlight, and Echo click effects scaled dynamically to cursor height and zoom level.
> 4. **Cursor Sway & Rotation**: Dynamic banking of cursor sprite by up to $\pm 15^\circ$ during high-speed movements.

### Answer
Analysis vs. Implementation Plan:

CURSOR_AND_ZOOM_ANALYSIS.md serves as the technical reference document defining theoretical cursor physics, anchor offsets, and zoom projection formulas. 

To implement these behaviors in ssA31, we will update the PlaybackEngine, ExportEngine, and MouseEvent data models.

Key Enhancements Included:
- Hotspot Anchor Alignment: Anchor ratios (anchorX, anchorY) for all 39 cursor types to guarantee zero tip displacement during cursor state swaps (e.g., Arrow <-> Hand <-> I-Beam).
- Sinusoidal Click Compression: 8% cursor compression applied over a sine curve upon click trigger.
- Proportional Vector Click Animations: Dynamic scaling of Ripple, Spotlight, and Echo click effects proportional to cursor height and active zoom level.
- Cursor Sway & Rotation: Dynamic banking of the cursor sprite by up to ±15° based on high-speed mouse velocity vectors.

Architecture & Target Components in ssA31:
1. MouseEvent.h: Add cursorType, anchorX/anchorY, velocityX/velocityY, and clickProgress telemetry fields.
2. PlaybackEngine.cpp: Maintain 39-cursor anchor lookup map and evaluate cursor sway rotation & frame interpolation.
3. ExportEngine.cpp: Render sinusoidal scale compression and vector click animations during compositing.



## Proposed Changes

### Core Input & Data Structures

#### [MODIFY] [`src/input/MouseEvent.h`](file:///Users/maitraprajapati/Desktop/ssA31/src/input/MouseEvent.h)
- Add hotspot anchor ratio fields `anchorX` and `anchorY` (normalized $[0.0, 1.0]$) to `MouseEvent`.
- Define cursor style scale multipliers for standard cursor styles (Tahoe, macOS, Windows 11, Dot, Figma).

---

### Playback & Camera Engine

#### [MODIFY] [`src/editor/PlaybackEngine.h`](file:///Users/maitraprajapati/Desktop/ssA31/src/editor/PlaybackEngine.h)
#### [MODIFY] [`src/editor/PlaybackEngine.cpp`](file:///Users/maitraprajapati/Desktop/ssA31/src/editor/PlaybackEngine.cpp)
- **Viewport Scale Calculation**: Implement `getCursorViewportScale(viewportWidth, minScale)` and `getCursorStyleMultiplier(style)` formulas from Section 2 of `CURSOR_AND_ZOOM_ANALYSIS.md`.
- **Click Bounce Compression**: Implement sinusoidal click progress scale formula:
  $$\text{bounceScale} = \max\left(0.72, 1.0 - \sin(t \cdot \pi) \cdot 0.08\right)$$
- **Cursor Sway Physics**: Calculate cursor velocity $(\text{vx}, \text{vy})$ between successive timestamps to derive directional sway rotation $\theta_{\text{sway}} \in [-15^\circ, +15^\circ]$.
- **Extended Cursor State Evaluation**: Update `evaluateCursorStateOffline()` to output anchor ratios, sway angle, bounce scale, and click effect parameters.

---

### Export & Compositing Engine

#### [MODIFY] [`src/export/ExportEngine.cpp`](file:///Users/maitraprajapati/Desktop/ssA31/src/export/ExportEngine.cpp)
- **Anchor Offset Math**: Replace static QPoint offset logic with precise anchor ratio math:
  $$\text{drawX} = \text{px} - (\text{anchorX} \cdot \text{drawWidth})$$
  $$\text{drawY} = \text{py} - (\text{anchorY} \cdot \text{drawHeight})$$
- **Click Bounce Rendering**: Apply `bounceScale` matrix transform on `QPainter` when rendering the active cursor image on click contact.
- **Enhanced Vector Click Effects**:
  - **Ripple**: Implement eased cubic expansion ($1 - (1-t)^3$) with dynamic radius $1.95 \cdot \text{scaledH}$ and decaying alpha.
  - **Spotlight**: Render concentric highlight rings around cursor tip during zoom focus.
  - **Echo**: Render multi-ring pulse wave around click origin.
- **Sway Rotation**: Rotate `QPainter` by `swayAngle` centered around the cursor hotspot.

---

### QML / Live Editor Rendering

#### [MODIFY] [`src/ui/EditorView.qml`](file:///Users/maitraprajapati/Desktop/ssA31/src/ui/EditorView.qml)
- Update QML canvas/cursor rendering overlay to apply the same anchor alignment math, click bounce compression, and ripple expansion for real-time editor playback.

---

## Verification Plan

### Automated Build & Unit Verification
- Compile the updated C++ codebase using CMake:
  ```bash
  cmake -B build -S .
  cmake --build build --config Release
  ```

### Manual Verification
1. **Dynamic Cursor State Swapping**: Test hovering over text fields (I-Beam), buttons (Pointing Hand), and default areas (Arrow) while zoomed in; verify cursor tip remains strictly locked to the target without jumping.
2. **Click Effects & Compression**: Perform mouse clicks while recording; verify 8% click bounce compression and smooth vector ripple/spotlight animations centered on the cursor tip.
3. **Export Verification**: Render an MP4 project with 2x zoom and verify high-DPI crisp rendering, motion-blurred cursor sway, and click effect synchronization.
