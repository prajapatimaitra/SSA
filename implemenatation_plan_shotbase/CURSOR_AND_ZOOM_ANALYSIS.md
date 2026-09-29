# Deep Technical Analysis: How the Cursor Works During Zoom-In (Scale, Click Effects & Dynamic Icons)

This document provides a detailed breakdown of how the cursor behaves during **Zoom-In** in Recordly — specifically explaining why and how the cursor appears enlarged, how dynamic cursor shapes (Arrow, Hand, I-Beam) stay accurately anchored, and how click effects (Ripples, Spotlights, Bounces) operate during zoomed playback.

---

## 1. How the Cursor Works During Zoom-In (Visual Mechanics)

When a video zooms in on a specific area of the screen (e.g., zooming into a code snippet, input box, or button), the cursor undergoes **4 key technical transformations**:

```
                  +-----------------------------------+
                  |   Camera Zoom In Event (1.8x-3x)  |
                  +-----------------+-----------------+
                                    |
            +-----------------------+-----------------------+
            |                                               |
            v                                               v
+-----------------------+                       +-----------------------+
| 1. Position Projection|                       | 2. Scale & Sizing     |
| Maps world (cx, cy)   |                       | Enlarges cursor height|
| to zoomed viewport    |                       | for viewer focus      |
+-----------+-----------+                       +-----------+-----------+
            |                                               |
            +-----------------------+-----------------------+
                                    |
            +-----------------------+-----------------------+
            |                                               |
            v                                               v
+-----------------------+                       +-----------------------+
| 3. Dynamic Icon Swap  |                       | 4. Click Effects      |
| Aligns hotspot anchors|                       | Scales ripple radius  |
| (Arrow/Hand/I-Beam)   |                       | & stroke proportionally|
+-----------------------+                       +-----------------------+
```

---

## 2. Why & How the Cursor is Enlarged (Scale Calculation)

In screen recordings, small cursor movements can easily get lost when zoomed in. Recordly calculates cursor size dynamically to keep the pointer prominent and legible.

### Sizing Formulas

1. **Base Pixel Height**:
   $$\text{h} = \text{dotRadius} \cdot \text{getCursorViewportScale}(\text{viewport.width}, \text{minViewportScale})$$
   Where $\text{dotRadius} = 28\text{px}$ by default, and `getCursorViewportScale` scales linearly relative to the $1920\text{px}$ reference viewport.

2. **Style Multiplier**:
   Different cursor styles (Tahoe, macOS, Windows 11, Dot, Figma) have different raw asset heights. Recordly applies a style scale multiplier:
   $$\text{scaledH} = \text{h} \cdot \text{getCursorStyleSizeMultiplier}(\text{style})$$

3. **Click Bounce Compression**:
   When a click happens, the cursor shrinks slightly by $8\%$ over a sinusoidal curve to mimic a physical mouse press:
   $$\text{bounceScale} = \max\left(0.72, 1 - \sin(\text{clickBounceProgress} \cdot \pi) \cdot (0.08 \cdot \text{clickBounce})\right)$$
   $$\text{drawHeight} = \text{scaledH} \cdot \text{bounceScale}$$
   $$\text{drawWidth} = \text{drawHeight} \cdot \text{aspectRatio}$$

---

## 3. Position Projection: Mapping Cursor to Zoomed Viewport

When camera zoom crops the video (e.g. zooming into top-right $30\%$ of the screen), the cursor's normalized coordinates $(cx, cy) \in [0, 1]$ are re-projected into zoomed viewport space via [`projectCursorPositionToViewport()`](file:///Users/maitraprajapati/Documents/Recordly-main/src/components/video-editor/videoPlayback/cursorViewport.ts):

$$\text{cx}_{\text{projected}} = \frac{\text{cx}_{\text{world}} - \text{crop.x}}{\text{crop.width}}$$
$$\text{cy}_{\text{projected}} = \frac{\text{cy}_{\text{world}} - \text{crop.y}}{\text{crop.height}}$$

$$\text{px}_{\text{screen}} = \text{viewport.x} + \text{cx}_{\text{projected}} \cdot \text{viewport.width}$$
$$\text{py}_{\text{screen}} = \text{viewport.y} + \text{cy}_{\text{projected}} \cdot \text{viewport.height}$$

This ensures that as the camera pans and zooms across the screen, the cursor stays pinned to the exact pixel of the recorded OS application.

---

## 4. How Dynamic Cursor Icons Work During Zoom-In (Arrow, Hand, I-Beam)

Even when zoomed in to 3x magnification, the cursor dynamically changes shape based on recorded telemetry stream:

| Cursor Type | Trigger Condition | Hotspot Anchor $(X, Y)$ | Zoom Behavior |
| :--- | :--- | :--- | :--- |
| **Arrow** (`arrow`) | Default pointer navigation | Top-left tip $(0.34, 0.24)$ | Enlarged arrow pointing directly at UI elements |
| **Pointing Hand** (`pointer`) | Hovering clickable buttons, links, tabs | Index finger tip $(0.39, 0.26)$ | Finger tip aligns exactly with click target |
| **I-Beam / Text** (`text`) | Hovering text inputs, selecting code/text | Center of vertical beam $(0.50, 0.50)$ | Beam aligns precisely between text characters |
| **Closed Hand** (`closed-hand`) | Dragging timeline, slider, or window | Center of palm $(0.50, 0.50)$ | Shows gripping action during drag operations |
| **Crosshair** (`crosshair`) | Precision crop, color picker, drawing | Center of cross $(0.50, 0.50)$ | Precision crosshair scaled cleanly |

### Hotspot Anchor Precision
Each cursor texture has custom anchor ratios (`anchorX`, `anchorY`). When swapping from an **Arrow** to an **I-Beam** or **Hand** during a zoomed view, the sprite's anchor point offsets the image drawing coordinates:

$$\text{drawX} = \text{px}_{\text{screen}} - (\text{anchorX} \cdot \text{drawWidth})$$
$$\text{drawY} = \text{py}_{\text{screen}} - (\text{anchorY} \cdot \text{drawHeight})$$

Because of this anchor offset math, switching cursor types during zoom-in **never causes the cursor tip to jump or shift position**.

---

## 5. How Click Effects Work During Zoom-In

Recordly renders procedural vector click animations around the cursor tip whenever a mouse click telemetry point is processed.

```
       Zoomed Cursor Tip (px, py)
                  |
      +-----------+-----------+
      |                       |
      v                       v
+------------------+   +------------------+
| Click Bounce     |   | Click Effect     |
| (Cursor shrinks  |   | Ring (Expands    |
|  by 8% on press) |   |  outward)        |
+------------------+   +------------------+
```

### 1. Ripple Effect (`ripple`)
- **Formula**:
  $$\text{easedProgress} = 1 - (1 - t)^3$$
  $$\text{rippleRadius} = \max(0.5, \text{easedProgress} \cdot \text{scaledH} \cdot 1.95 \cdot \text{effectScale})$$
  $$\text{alpha} = \max(0, \min(1, t^3 \cdot 0.6 \cdot \text{effectOpacity}))$$
  $$\text{strokeWidth} = \max(1, 2 \cdot t^3)$$
- **Zoom Behavior**: Because `rippleRadius` and `strokeWidth` scale directly with `scaledH` (the enlarged cursor height), the ripple ring expands gracefully around the enlarged cursor without looking thin or tiny.

### 2. Spotlight Effect (`spotlight`)
- Renders dual concentric glowing rings around the cursor tip to highlight the clicked button or text box during zoomed tutorials.
- `glowRadius = baseRadius + (1 - t) * scaledH * effectScale`
- `innerRadius = max(baseRadius * 0.72, glowRadius * 0.76)`

### 3. Echo Effect (`echo`)
- Renders triple layered pulse rings with a solid center dot highlight.

---

## 6. Physics & Visual Enhancements During Zoom

1. **Spring Motion Physics (`SmoothedCursorState`)**:
   - Uses spring physics (stiffness, damping, mass) to smooth out micro-tremors from hand movements:
     $$v_{t+1} = (v_t + \text{stiffness} \cdot (\text{target} - \text{current}) \cdot dt) \cdot \text{damping}$$
     $$\text{current}_{t+1} = \text{current}_t + v_{t+1} \cdot dt$$
2. **Dynamic Sway Rotation**:
   - When the cursor moves quickly across a zoomed viewport, `computeCursorSwayRotation()` rotates (banks) the cursor sprite by up to $\pm 15^\circ$ in the direction of travel.
3. **High-DPI Texture Normalization**:
   - System cursor images are rasterized to a reference pixel height of $1024\text{px}$ ([`SystemCursorAssets.swift`](file:///Users/maitraprajapati/Documents/Recordly-main/electron/native/SystemCursorAssets.swift)).
   - Even under extreme camera zoom-in ($10\text{x}$ magnification), the cursor texture remains crisp with zero pixelation or blurry edges.

---

## 7. Summary Table: Cursor Attributes During Zoom

| Feature | Behavior During Zoom-In | Implementation File |
| :--- | :--- | :--- |
| **Cursor Size** | Enlarged dynamically via `scaledH` & viewport width scaling | [`cursorRenderer.ts`](file:///Users/maitraprajapati/Documents/Recordly-main/src/components/video-editor/videoPlayback/cursorRenderer.ts#L1449) |
| **Cursor Position** | Projected relative to cropped camera frame `(crop.x, crop.y, crop.width, crop.height)` | [`cursorViewport.ts`](file:///Users/maitraprajapati/Documents/Recordly-main/src/components/video-editor/videoPlayback/cursorViewport.ts#L21) |
| **Click Ring Effect** | Expands radially outward scaled to cursor height | [`cursorRenderer.ts`](file:///Users/maitraprajapati/Documents/Recordly-main/src/components/video-editor/videoPlayback/cursorRenderer.ts#L879) |
| **Click Bounce** | 8% height compression on click contact | [`cursorRenderer.ts`](file:///Users/maitraprajapati/Documents/Recordly-main/src/components/video-editor/videoPlayback/cursorRenderer.ts#L1445) |
| **Dynamic Shape Swap** | Switches Arrow $\leftrightarrow$ Hand $\leftrightarrow$ I-Beam with zero anchor displacement | [`uploadedCursorAssets.ts`](file:///Users/maitraprajapati/Documents/Recordly-main/src/components/video-editor/videoPlayback/uploadedCursorAssets.ts#L85) |
| **Motion Blur & Sway** | Adds directional blur filter and rotational banking in speed movements | [`zoomTransform.ts`](file:///Users/maitraprajapati/Documents/Recordly-main/src/components/video-editor/videoPlayback/zoomTransform.ts#L261) |
