# Phase 10: GPU Renderer

I saw in the logs that you were clicking the "Export" button! Let's build the engine to make it happen!

As defined in your master blueprint, before we can export the `.mp4` file (Phase 11), we must build the **GPU Renderer** (Phase 10). This is the engine that actually composites the raw video frame, applies the mathematical camera zoom transformations, and draws the cursor overlay into a final flattened image.

## Goal
Establish the `IRenderer` abstraction and build the `MetalRenderer` implementation for macOS.

## Proposed Changes

### Core Interfaces
#### [NEW] [IRenderer.h](file:///Users/maitraprajapati/Desktop/ssA/src/renderer/IRenderer.h)
- Define the `IRenderer` interface:
  - `initialize(int outputWidth, int outputHeight)`
  - `renderFrame(CVPixelBufferRef videoFrame, double cameraX, double cameraY, double cameraZoom, double cursorX, double cursorY, int cursorState)`
  - `CVPixelBufferRef getOutputTexture()`

### Metal Backend (macOS)
#### [NEW] [MetalRenderer.h](file:///Users/maitraprajapati/Desktop/ssA/src/renderer/macos/MetalRenderer.h) & [MetalRenderer.mm](file:///Users/maitraprajapati/Desktop/ssA/src/renderer/macos/MetalRenderer.mm)
- Implements `IRenderer` using Apple's Metal framework.
- Creates an offscreen `MTLTexture` as the render target.
- Sets up an `MTLCommandQueue` and `MTLRenderPipelineState`.
- For each frame, it draws a textured quad (the video frame) applying a translation and scale matrix based on the `CameraEngine`'s math.
- It then draws the cursor dot using a secondary shader pass over the scaled video.

#### [NEW] [Shaders.metal](file:///Users/maitraprajapati/Desktop/ssA/src/renderer/macos/Shaders.metal)
- `vertex_main`: Applies the 2D zoom/pan transformation matrix.
- `fragment_main`: Samples the video texture.
- `fragment_cursor`: Procedurally draws the red circular cursor dot.

### CMake Integration
#### [MODIFY] [CMakeLists.txt](file:///Users/maitraprajapati/Desktop/ssA/src/CMakeLists.txt)
- Link against Apple's `Metal` and `MetalKit` frameworks.

## Open Questions

> [!CAUTION]
> **Architectural Crossroads!** 
> 
> Writing a raw Metal rendering pipeline from scratch (compiling shaders, managing vertex buffers, texture caching) is highly performant but very complex and time-consuming.
> 
> **Alternative Option:** Since we have already built the UI perfectly in Qt QML (with the zooms and cursor), we can use **Qt's `QQuickRenderControl`**. This allows us to instantiate your `EditorView.qml` *off-screen*, fast-forward the timeline frame-by-frame, and let Qt's internal graphics engine do all the compositing for us! 
> 
> **Do you want me to stick strictly to the blueprint (Raw C++ Metal Renderer), or should we pivot to using `QQuickRenderControl` for off-screen UI rendering to save hours of development time?**
