# Implementation Plan: Phase 4 (Mouse and Click Metadata)

This plan outlines the architecture and strategy for tracking precise mouse coordinates and clicks synchronously with our screen capture frames.

## User Review Required

> [!IMPORTANT]
> Just like in Phase 3, we will implement the fully functional tracking logic for macOS using `CGEventTaps` because we are on a Mac environment. For Windows and Linux, we will provide safe C++ compilation stubs.
> 
> Furthermore, in macOS, listening to global mouse events requires **Accessibility Permissions**. Running the app will prompt for this alongside Screen Recording. Is this acceptable?

## Proposed Changes

We will introduce a platform-independent input tracking interface, and a robust coordinate mapper.

### Core Data Structures & Interfaces

#### [NEW] [src/input/MouseEvent.h](file:///Users/maitraprajapati/Desktop/ssA/src/input/MouseEvent.h)
Defines the `MouseEvent` data structure:
- `timestamp` (uint64_t in microseconds)
- `x`, `y` (double coordinates)
- `button` (enum: Left, Right, Middle, None)
- `type` (enum: Move, Down, Up)

#### [NEW] [src/input/interfaces/IInputTracker.h](file:///Users/maitraprajapati/Desktop/ssA/src/input/interfaces/IInputTracker.h)
Defines the `IInputTracker` interface:
- `virtual void startTracking() = 0;`
- `virtual void stopTracking() = 0;`
- `virtual void setEventCallback(std::function<void(const MouseEvent&)> callback) = 0;`

#### [MODIFY] [src/platform/PlatformFactory.h](file:///Users/maitraprajapati/Desktop/ssA/src/platform/PlatformFactory.h)
Add `std::unique_ptr<input::IInputTracker> createInputTracker();`

### Coordinate System
#### [NEW] [src/core/coordinates/CoordinateMapper.h](file:///Users/maitraprajapati/Desktop/ssA/src/core/coordinates/CoordinateMapper.h)
#### [NEW] [src/core/coordinates/CoordinateMapper.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/core/coordinates/CoordinateMapper.cpp)
Provides pure functions to convert OS-level coordinates into Capture-level coordinates, correcting for display DPI and origin offsets on multi-monitor setups.

### macOS Implementation (CGEventTap)

#### [NEW] [src/input/macos/MacInputTracker.mm](file:///Users/maitraprajapati/Desktop/ssA/src/input/macos/MacInputTracker.mm)
Uses macOS `CGEventTapCreate` to intercept global mouse events (Move, Drag, Left/Right Down/Up).
- Translates `CGEvent` timestamps and coordinates into our platform-neutral `MouseEvent`.
- Requires the application to be granted Accessibility API permissions.

### Windows & Linux Implementations (Stubs)

#### [NEW] [src/input/windows/WindowsInputTracker.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/input/windows/WindowsInputTracker.cpp)
Safe compilation stub for Windows. (Eventual implementation would use `SetWindowsHookEx(WH_MOUSE_LL)`).

#### [NEW] [src/input/linux/LinuxInputTracker.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/input/linux/LinuxInputTracker.cpp)
Safe compilation stub for Linux.

### Integration

#### [MODIFY] [src/capture/CaptureController.h](file:///Users/maitraprajapati/Desktop/ssA/src/capture/CaptureController.h)
#### [MODIFY] [src/capture/CaptureController.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/capture/CaptureController.cpp)
- Takes ownership of the `IInputTracker` engine.
- Starts/stops input tracking in tandem with video capture.
- Will log mouse clicks alongside the video frame captures to verify synchronization.

#### [MODIFY] [src/CMakeLists.txt](file:///Users/maitraprajapati/Desktop/ssA/src/CMakeLists.txt)
Include the new `input/` directory headers and sources in the build configuration.

## Verification Plan

### Automated Tests
- Build system verification across platforms (APPLE, WIN32, UNIX).

### Manual Verification
- Launch the application and click "Start Recording".
- Allow Accessibility Permissions if prompted.
- Move the mouse around and click on the screen.
- Verify the console logs: `[INFO] Mouse Click Down at X: 450.5, Y: 1020.2 (Timestamp: ...)` alongside video frame logs.
