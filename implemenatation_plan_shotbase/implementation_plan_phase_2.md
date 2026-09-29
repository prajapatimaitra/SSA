# Goal Description
The objective of **Phase 2: Global Hotkeys** is to allow users to trigger core recording actions (Start, Stop, Screenshot) even when the application is running in the background and does not have focus. 

Since Qt does not natively support system-wide global hotkeys (its `QShortcut` only works when the application is active), we need to build a custom C++ layer that integrates directly with native OS accessibility and event monitor APIs.

## Open Questions
- Do you want to use the default macOS screenshot shortcut (`Cmd+Shift+5`), or should we default to a custom shortcut like `Cmd+Shift+R` to avoid conflicting with Apple's native ScreenCapture utility?

## Proposed Changes

### Core Interface
We will introduce a platform-agnostic interface that our application can use without worrying about the underlying operating system.

#### [NEW] `src/workspace/interfaces/IGlobalShortcutProvider.h`
- Defines the `IGlobalShortcutProvider` class with pure virtual methods: `registerShortcut(...)` and `unregisterShortcut(...)`.
- Defines an enum for standard keys and modifiers to abstract away OS-specific key codes.

### macOS Implementation
Apple requires using `NSEvent addGlobalMonitorForEventsMatchingMask` for listening to keystrokes outside the app.

#### [NEW] `src/workspace/macos/MacShortcutProvider.mm`
- Implements `IGlobalShortcutProvider`.
- Hooks into the macOS event loop via Objective-C++.
- Triggers standard C++ callbacks when the registered key combination is pressed.

### Controller Integration
We need a manager to tie the physical shortcuts to the actual `CaptureController`.

#### [NEW] `src/workspace/ShortcutManager.h` & `ShortcutManager.cpp`
- Creates the correct `IGlobalShortcutProvider` based on the current platform (e.g., `MacShortcutProvider`).
- Registers the default shortcuts on startup.
- Connects the shortcut triggers directly to `CaptureController::startCapture()` and `CaptureController::stopCapture()`.

#### [MODIFY] `src/app/main.cpp`
- Instantiate the `ShortcutManager` and pass it the `CaptureController`.

#### [MODIFY] `src/CMakeLists.txt`
- Add the new C++ and Objective-C++ files to the build system.

## Verification Plan

### Manual Verification
1. Launch the application.
2. Click away from the application so it loses focus.
3. Press the global shortcut (e.g., `Cmd+Shift+R`).
4. Verify that the System Tray icon turns red (indicating recording started).
5. Press the shortcut again and verify the recording stops.



# The Shortcuts
To avoid conflicting with macOS native screenshot utilities (like Cmd+Shift+5 or Cmd+Shift+R), I configured the following unique combinations:

Start / Stop Recording: Cmd + Option + Shift + R
## Take Screenshot: Cmd + Option + Shift + S (This currently prints a debug log until we build Phase 3!)
How to Test It
The application is currently running in the background!

Click away from the application so another app (like your browser) is active.
## Press Cmd + Option + Shift + R.
Watch the System Tray icon in your Mac Menu Bar turn red, indicating recording has started!
## Press Cmd + Option + Shift + R again.
Watch the recording stop and the Editor window open!