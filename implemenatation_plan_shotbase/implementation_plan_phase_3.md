# Implementation Plan Phase 3: Screenshot Workspace

This phase introduces a completely new capability to our workspace: capturing and editing static screenshots, triggered directly via our global hotkey (`Cmd+Option+Shift+S`) or the System Tray.

## Goal Description
Build a lightweight "Screenshot Editor" view into the application. When a user captures a screenshot, they shouldn't just get a file dumped on their desktop; they should be seamlessly thrown into an editor where they can annotate it (draw arrows, rectangles) and instantly copy it to their clipboard.

## Proposed Changes

### 1. Capture Engine Extensions
The core `CaptureController` is currently hardcoded for video. We will add a dedicated static frame capture method.

#### [MODIFY] [CaptureController.h](file:///Users/maitraprajapati/Desktop/ssA/src/capture/CaptureController.h)
- Add `Q_INVOKABLE void captureScreenshot()`.
- Add `signal: void screenshotCaptured(const QString& imagePath)`.

#### [MODIFY] [CaptureController.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/capture/CaptureController.cpp)
- Implement `captureScreenshot()`.
- Wait, since we are using `IPlatformCapture` (which wraps Apple's ScreenCaptureKit), doing a static grab requires a slightly different approach or just grabbing the very first frame of a stream and immediately stopping it. To keep it robust and decoupled from video logic, we might use a direct Qt screen grab (`QGuiApplication::primaryScreen()->grabWindow(0)`) or a lightweight native call for screenshots.

### 2. Shortcut Wiring
We already set up `Cmd+Option+Shift+S` in Phase 2, but it currently just prints a log.

#### [MODIFY] [ShortcutManager.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/workspace/ShortcutManager.cpp)
- Change the log to call `m_captureController->captureScreenshot()`.

### 3. Screenshot Editor UI
We need a brand new QML view for editing static images, distinct from the video `EditorView.qml`.

#### [NEW] [ScreenshotEditor.qml](file:///Users/maitraprajapati/Desktop/ssA/ui/ScreenshotEditor.qml)
- An `Image` component to display the captured screenshot.
- A `Canvas` overlay to handle drawing logic (Rectangles, Arrows, Freehand).
- A Toolbar at the top for selecting tools (Pen, Rectangle, Color).
- A "Copy to Clipboard" button.

### 4. Clipboard Controller (C++)
QML cannot natively push image data (like a Canvas rendering) directly to the OS clipboard easily without C++ help.

#### [NEW] [ClipboardManager.h](file:///Users/maitraprajapati/Desktop/ssA/src/workspace/ClipboardManager.h)
#### [NEW] [ClipboardManager.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/workspace/ClipboardManager.cpp)
- Add `Q_INVOKABLE void copyImageToClipboard(const QString& localFileUrl)` that uses `QClipboard`.
- Expose this manager to the QML engine in `main.cpp`.

### 5. Application State & Navigation
The app needs to know when to show the Video Editor vs the Screenshot Editor.

#### [MODIFY] [MainWindow.qml](file:///Users/maitraprajapati/Desktop/ssA/ui/MainWindow.qml)
- Add the `ScreenshotEditor` to the `StackLayout`.
- Listen to `captureController.screenshotCaptured` and switch the state/index to the Screenshot Editor.

## Verification Plan

### Manual Verification
1. **Trigger:** Press `Cmd+Option+Shift+S`.
2. **Capture:** Verify a screenshot is successfully taken and saved to disk.
3. **UI Transition:** Verify the app instantly switches from hidden/background to the new `ScreenshotEditor.qml`.
4. **Annotation:** Draw a red rectangle on the canvas.
5. **Clipboard:** Click "Copy to Clipboard", open Apple Notes (or Preview), and hit Paste (`Cmd+V`). Verify the annotated image pastes successfully.

## Open Questions
> [!NOTE]
> Qt provides `QGuiApplication::primaryScreen()->grabWindow(0)` which is incredibly fast and reliable for taking screenshots across all platforms. Should we use this standard Qt method for screenshots, or do you specifically want to use native Apple APIs (like we did with ScreenCaptureKit for video) to capture the screenshot? (Using Qt is highly recommended here for cross-platform simplicity and speed).

# Architectural Decision: Cross-Platform Screenshots

## The Decision

**We will use the Native OS APIs (ScreenCaptureKit, Windows Graphics Capture, PipeWire) to capture screenshots.**

Using the native, OS-level tools provided by Apple, Microsoft, and Linux ensures that our application captures screenshots with the highest possible performance, reliability, and visual fidelity, while fully respecting modern OS security boundaries.

## The Cross-Platform Strategy ("The Interface Pattern")

Using "Native APIs" does not mean we are building exclusively for Mac, nor does it mean we are sacrificing cross-platform compatibility. Instead, we use the **Interface Pattern**:

1. **The Core Interface:** We define a single, platform-agnostic C++ interface (e.g., `IScreenCapturer`). The rest of the application (UI, editing logic) only talks to this interface and doesn't care what operating system it is running on.
2. **Native Implementations:** We write three specific, native implementations "under the hood":
   * **Mac:** `MacScreenCapturer.cpp` (using ScreenCaptureKit)
   * **Windows:** `WindowsScreenCapturer.cpp` (using Windows Graphics Capture)
   * **Linux:** `LinuxScreenCapturer.cpp` (using PipeWire)
3. **CMake Compilation:** When the application is compiled, **CMake** automatically detects the user's OS and only builds the correct native file. 

The result is a single codebase that safely and efficiently executes perfect, native code across Windows, Mac, and Linux.

## Benefits of Native APIs

### 1. Modern OS Security & Permissions
Modern operating systems heavily restrict screen capturing to protect user privacy.
* **macOS:** Apple requires explicit "Screen Recording" permissions. ScreenCaptureKit integrates perfectly with these prompts and guarantees capture success once granted.
* **Linux:** The modern Wayland display server prevents apps from reading pixels outside their own windows. Native APIs (like PipeWire and `xdg-desktop-portal`) are the *only* way to bypass this security to capture the whole desktop.

### 2. Multi-Monitor and Mixed DPI Accuracy
On modern setups with multiple monitors at different scaling factors (e.g., a 4K display at 150% scaling next to a 1080p display at 100%), taking an accurate screenshot is mathematically complex. Native APIs handle OS-level compositor scaling perfectly, ensuring the resulting screenshot is never distorted or incorrectly cropped.

### 3. Reusing Existing Architecture
Since the application already utilizes native APIs (like ScreenCaptureKit) for capturing *video*, we have already handled the complex setup and permission flows. Grabbing a screenshot is as simple as asking our existing, robust native pipeline to emit a single frame, saving significant development overhead.

