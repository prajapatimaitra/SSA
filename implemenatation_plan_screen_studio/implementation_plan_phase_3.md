# Implementation Plan: Phase 3 (Screen Capture)

This plan outlines the architecture and implementation strategy for Phase 3: Hardware-accelerated screen capture.

## User Review Required

> [!IMPORTANT]
> Since we are developing on macOS, we can fully implement and test the **ScreenCaptureKit** logic for Apple Silicon. 
> For Windows and Linux, I will provide the structural C++ classes and CMake wiring, but their internal logic will remain as safe stubs (returning empty frames or logging warnings) for now to ensure we don't break cross-platform compilation without being able to test it. We can fill in the WinRT and PipeWire logic later when building on those specific environments. Is this approach acceptable?

## Proposed Changes

We will introduce the `IPlatformCapture` interface and a platform-neutral `VideoFrame` representation. 

### Core Abstractions

#### [NEW] [src/media/VideoFrame.h](file:///Users/maitraprajapati/Desktop/ssA/src/media/VideoFrame.h)
Defines the `VideoFrame` struct.
- Contains: `timestamp`, `width`, `height`, `PixelFormat` enum (e.g., BGRA, RGBA, NV12).
- Contains: An opaque `void* nativeBuffer` (which will hold `CVPixelBufferRef` on Mac).

#### [NEW] [src/capture/interfaces/IPlatformCapture.h](file:///Users/maitraprajapati/Desktop/ssA/src/capture/interfaces/IPlatformCapture.h)
Abstract interface for controlling screen recording.
- `virtual void startCapture(int displayId) = 0;`
- `virtual void stopCapture() = 0;`
- `virtual void setFrameCallback(std::function<void(const VideoFrame&)> callback) = 0;`

#### [MODIFY] [src/platform/PlatformFactory.h](file:///Users/maitraprajapati/Desktop/ssA/src/platform/PlatformFactory.h)
Add `std::unique_ptr<IPlatformCapture> createScreenCapture();`

### macOS Implementation (ScreenCaptureKit)

#### [NEW] [src/capture/macos/MacScreenCapture.h](file:///Users/maitraprajapati/Desktop/ssA/src/capture/macos/MacScreenCapture.h)
#### [NEW] [src/capture/macos/MacScreenCapture.mm](file:///Users/maitraprajapati/Desktop/ssA/src/capture/macos/MacScreenCapture.mm)
Implements `IPlatformCapture` using macOS 12+ `ScreenCaptureKit` (`SCStream`).
- Will implement `SCStreamOutput` delegate to receive sample buffers.
- Converts `CMSampleBufferRef` to our platform-neutral `VideoFrame` and fires the callback.

### Windows & Linux Implementations (Stubs)

#### [NEW] [src/capture/windows/WindowsGraphicsCapture.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/capture/windows/WindowsGraphicsCapture.cpp)
Safe compilation stub for Windows.

#### [NEW] [src/capture/linux/PipeWireCapture.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/capture/linux/PipeWireCapture.cpp)
Safe compilation stub for Linux.

### UI Integration

#### [MODIFY] [src/app/main.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/app/main.cpp)
- Wire up the capture engine.
- Expose a simple start/stop recording slot to QML.

#### [MODIFY] [src/ui/MainWindow.qml](file:///Users/maitraprajapati/Desktop/ssA/src/ui/MainWindow.qml)
- Connect the "Start Recording" button to the C++ capture engine.
- For this phase, we won't render the video frame to the UI yet (that's Phase 6 / 10), but we will log a message every time a frame is successfully captured to prove the pipeline works.

## Verification Plan

### Automated Tests
- CMake will verify the new files compile under `APPLE`, `WIN32`, and `UNIX`.

### Manual Verification
- Click "Start Recording" in the UI.
- Verify that macOS prompts for Screen Recording permissions (if not already granted).
- Verify that the application console rapidly logs `[INFO] Captured frame: 1920x1080 (Format: BGRA)` proving that `ScreenCaptureKit` is actively streaming frames to our C++ core.
