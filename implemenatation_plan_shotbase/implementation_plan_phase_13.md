# Phase 13: Batch Export Queue

Currently, the `ExportEngine` is tightly coupled to the `EditorController`. This means you can only export the project you are currently looking at, and if you change settings while it's exporting, it could corrupt the render.

We will decouple the rendering engine and build a headless `ExportQueueManager`. This allows you to queue up multiple projects for rendering and continue working on other projects simultaneously!

## Proposed Changes

### 1. Decoupling the Export Engine
#### [MODIFY] `src/export/ExportEngine.h` & `.cpp`
- Remove the `EditorController*` dependency.
- Change `startExport` to take `(projectPath, presetName, resolution)`.
- Inside `ExportEngine`, it will silently use `MetadataSerializer` to load the `metadata.json` from disk, construct a headless `PlaybackEngine` just for the render, and pull all URLs, trim bounds, and camera keyframes completely offline.

### 2. The Export Queue Manager
#### [NEW] `src/export/ExportQueueManager.h` & `.cpp`
- A Singleton QObject that manages a FIFO queue of `ExportJob`s.
- Exposes `enqueueExport(projectPath, presetName, resolution)` to QML.
- Exposes properties like `isExporting`, `currentProjectName`, and `currentProgress` to drive the UI.
- Automatically pops the next job and feeds it to `ExportEngine` when the previous one finishes.

#### [MODIFY] `src/CMakeLists.txt`
- Add `ExportQueueManager.cpp` to the build.

#### [MODIFY] `src/app/main.cpp`
- Instantiate `ExportQueueManager` and expose it to the QML root context as `exportQueue`.

### 3. Editor & UI Updates
#### [MODIFY] `src/editor/EditorController.cpp`
- Change `exportVideo` to simply call `exportQueue->enqueueExport(...)` instead of directly managing the engine.

#### [MODIFY] `src/ui/EditorView.qml`
- Add a new visual indicator in the Top Bar that shows "Rendering: X%" when the background queue is active.
- Change the Export button to "Add to Render Queue".

## Verification Plan
1. `cd build && make -j8`
2. Open a project in the Editor and click "Add to Render Queue".
3. Verify that the top bar shows "Rendering: 0...100%".
4. Immediately go back to the Library, open a *different* project, and click "Add to Render Queue" while the first one is still going.
5. Verify that it queues it up seamlessly and renders the second one correctly right after!
