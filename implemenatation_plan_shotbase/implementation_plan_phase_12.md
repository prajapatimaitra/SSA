# Phase 12: Native Sharing & File System Interface

This phase introduces direct integration with the operating system's native file explorer and sharing menus. Currently, "Reveal in Finder" in the Library merely opens the containing folder. We will build a native macOS pipeline that highlights the exact file and allows instant sharing via the macOS Share Sheet (AirDrop, Messages, Mail).

## Proposed Changes

### 1. New C++ Interfaces
We will create a new architecture in `src/workspace` to handle native OS calls:
#### [NEW] `src/workspace/interfaces/INativeIntegration.h`
- Defines `virtual void revealInOS(const QString& path) = 0;`
- Defines `virtual void shareFile(const QString& path) = 0;`

#### [NEW] `src/workspace/NativeIntegrationManager.h` & `.cpp`
- A QObject singleton exposed to QML as `nativeIntegration`.
- Owns a unique pointer to the platform-specific `INativeIntegration` implementation.

### 2. macOS Implementation
#### [NEW] `src/workspace/macos/MacNativeIntegration.h` & `.mm`
- **`revealInOS`**: Implements `[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:]` to open Finder and immediately highlight the exact file.
- **`shareFile`**: Implements `NSSharingServicePicker` to spawn the native macOS Share Sheet (AirDrop, Mail, Messages, etc.) right from the application window.

### 3. CMake Updates
#### [MODIFY] `src/CMakeLists.txt`
- Add the new `NativeIntegration` sources to the CMake configuration.

### 4. QML Integration
#### [MODIFY] `src/app/main.cpp`
- Register `NativeIntegrationManager` as a QML root context property.

#### [MODIFY] `src/ui/components/MediaCard.qml`
- Update the "Reveal in Finder" context menu item to call `nativeIntegration.revealInOS(root.path)` instead of the crude `Qt.openUrlExternally`.
- Add a new "Share..." context menu item that calls `nativeIntegration.shareFile(root.path)`.

#### [MODIFY] `src/ui/EditorView.qml`
- Add a "Share" button to the top bar (next to Export) that allows sharing the exported video directly from the editor once an export completes!

## Verification Plan
1. Recompile the application (`cd build && make -j8`).
2. Open the Library, right-click a recording, and click "Reveal in Finder." Verify that Finder opens and the *exact file is visually highlighted*.
3. Right-click a recording and click "Share...". Verify that the native macOS Share Sheet appears.
