# Goal Description
The objective of **Phase 1: System Tray & Global Capture Menu** is to provide a unified capture launcher available globally from the OS menu bar (macOS) or system tray (Windows). 

Users should be able to quickly choose between starting a screen recording, taking a screenshot, and quitting the application, all while the main window is hidden.

## Open Questions
- None. (This phase has already been successfully executed!)

## Proposed Changes

### Controller Integration
We will introduce a new C++ controller that manages the native `QSystemTrayIcon` and hooks it into our existing recording infrastructure.

#### [NEW] `src/workspace/SystemTrayController.h` & `SystemTrayController.cpp`
- Creates a `QSystemTrayIcon` and a native `QMenu`.
- Adds actions: "Record Screen", "Screenshot", and "Quit".
- Connects directly to the existing `CaptureController::startCapture()` and `stopCapture()` methods.
- Listens to new state signals to update the tray icon (e.g., changing the icon to a red dot when actively recording).

#### [MODIFY] `src/capture/CaptureController.h` & `CaptureController.cpp`
- Add `recordingStarted()` and `recordingStopped()` signals so the System Tray can react to state changes without tightly coupling the two systems.

#### [MODIFY] `src/app/main.cpp`
- Change `QGuiApplication` to `QApplication` (since system tray icons use QtWidgets under the hood in Qt).
- Instantiate the `SystemTrayController` on app launch.

#### [MODIFY] `src/CMakeLists.txt`
- Add `Qt6::Widgets` to the linked libraries to support `QSystemTrayIcon`.
- Add the new `SystemTrayController` source and header files.

## Verification Plan

### Manual Verification
1. Launch the application.
2. Verify the system tray icon appears in the macOS Menu Bar.
3. Click the icon to open the native menu.
4. Click "Record Screen".
5. Verify the icon turns into a red dot indicating recording is active.
6. Click the menu again and select "Stop Recording".
7. Verify the icon returns to the default state and the editor window opens correctly with the recorded video.
