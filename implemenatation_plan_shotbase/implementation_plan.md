# Goal Description
The objective is to extend the existing Screen Studio Alternative (`ssA`) C++ / Qt 6 application into a complete "Shotbase-style" workspace. This means taking our existing, stable screen recording, editing, and rendering foundation, and wrapping a powerful organizational and productivity layer around it.

> [!IMPORTANT]  
> The absolute primary rule of this implementation is to **preserve the existing recording and editor engine**. We will not rewrite or replace our existing Qt/C++ FFmpeg/Webcam/Compositing architecture. Every new feature must seamlessly integrate into the existing core.

## Feature Audit
Before writing code, here is the audit of the current existing application versus the requested Shotbase features:

| Feature | Status |
| :--- | :--- |
| **Recorder / Editor / Timeline / Render Engine** | ✅ Existing & Protected |
| **Global Capture Menu** | ❌ Missing |
| **Global Hotkeys** | ❌ Missing |
| **System Tray / Menu Bar** | ❌ Missing |
| **Screenshot Workflow** | ❌ Missing |
| **Library (Local Workspace)** | ❌ Missing |
| **Search, Tags, Collections** | ❌ Missing |
| **Auto Save & Crash Recovery** | ❌ Missing |
| **Templates & Style Presets** | ❌ Missing |
| **Recording & Export Presets** | ❌ Missing |
| **Batch Export** | ❌ Missing |
| **Native Sharing & File System Integrations** | ❌ Missing |
| **Command Palette & Context Menus** | ❌ Missing |
| **Markers, Notes, Chapters** | ❌ Missing |
| **Redaction (Blur/Pixelate Layer)** | ❌ Missing |
| **Application Settings & Privacy Center** | ❌ Missing |

## Detailed Implementation Phases

### TIER 1 — CORE WORKSPACE
#### Phase 1: System Tray & Global Capture Menu
- **What is being built:** A unified capture launcher available globally from the system tray (Windows) or Menu Bar (macOS). Users can quickly choose between Screenshot, Record Region, Record Window, and Record Display.
- **Existing Systems Reused:** `CaptureController.h/cpp` (We will reuse the exact same recording endpoints).
- **New Components:** `SystemTrayController.h` (C++ wrapper using `QSystemTrayIcon`), and a lightweight QML popup window for the capture menu.

#### Phase 2: Global Hotkeys
- **What is being built:** A configurable, system-wide shortcut system that allows users to start/stop/pause recording and take screenshots even when the app is in the background.
- **Existing Systems Reused:** The actual start/stop recording mechanisms in `CaptureController`.
- **New Components:** `IGlobalShortcutProvider` interface with platform-specific implementations (using `RegisterHotKey` on Windows, `NSEvent` on macOS) integrated into the Qt Event Loop.

#### Phase 3: Screenshot Workspace
- **What is being built:** Treating screenshots as first-class citizens. Capturing a screenshot instantly opens a dedicated annotation editor.
- **Existing Systems Reused:** `ScreenCaptureKit` / `QScreen` logic, bypassing the video encoder.
- **New Components:** An Annotation Preset system (Arrows, Rectangles, Text) and a lightweight `ScreenshotEditorView.qml`.

#### Phase 4: Local Media Library (SQLite)
- **What is being built:** A high-performance, disk-based catalog of all projects, recordings, and screenshots.
- **Existing Systems Reused:** The `.ssa` project bundle system.
- **New Components:** A C++ `LibraryManager` utilizing `QSqlDatabase` (SQLite) to store metadata, paths, thumbnails, and timestamps for fast retrieval.

#### Phase 5: Workspace UI (Tags, Search, Recent)
- **What is being built:** A unified "Home" screen for the application featuring Grid/List views, fast local text search, user-defined Tags, and Collections.
- **Existing Systems Reused:** The main application window routing.
- **New Components:** `LibraryView.qml` with virtualized lists (`GridView`/`ListView`) for high performance, backed by a `QAbstractListModel` connected to the SQLite database.

#### Phase 6: Auto-Save & Crash Recovery
- **What is being built:** Background protection against data loss. Automatically saves the state of the editor timeline, zooms, and styles continuously.
- **Existing Systems Reused:** `ProjectManager::serializeProject()`.
- **New Components:** A `RecoveryManager` that writes atomic JSON snapshots to a temporary cache. On startup, if an un-closed snapshot is detected, it prompts the user to restore.

---

### TIER 2 — PRODUCTIVITY & SPEED
#### Phase 7: Project Import & Drag and Drop
- **What is being built:** The ability to drag external MP4s or PNGs into the app, creating an `.ssa` project wrapping that asset instantly.
- **Existing Systems Reused:** `IMediaDecoder` and `FFmpegDecoder`.
- **New Components:** QML `DropArea` handling, path validation, and media validation to prevent corrupted files from crashing the editor.

#### Phase 8: Project Duplication & Templates
- **What is being built:** Allowing users to reuse their best layouts. Duplicating a project without duplicating the massive underlying video file.
- **Existing Systems Reused:** The Project JSON structure.
- **New Components:** A Template engine that extracts Background Color, Aspect Ratio, Cursor Styles, and Padding, saving them as independent, reusable `.json` assets.

#### Phase 9: Style, Recording, and Export Presets
- **What is being built:** Quick-start configuration profiles (e.g., "YouTube Export", "Developer Dark Theme", "Vertical Recording"). This includes robust **Recording Presets**:
  - **Target Framerate (FPS):** Options for 30 FPS (standard) or 60 FPS (crucial for smooth cursor movements and high-end tutorials).
  - **Target Capture Resolution:** Since disk space is not a concern, offer a full range of resolutions for the raw capture (the app will scale the raw screen to this size while recording).
  - **Audio Source Selection:** A dropdown with three options: 1. Microphone Only, 2. System Audio Only (Desktop sounds), 3. Both (Mic + System Audio). When "Both" is selected, the application will record them simultaneously into two separate audio files (e.g., `system.wav` and `mic.wav`) within the project bundle for non-destructive editing.
- **Existing Systems Reused:** `ExportEngine`, `CaptureController`, and `EditorController` configuration setters.
- **New Components:** Preset storage layer in SQLite, passing predefined arguments into the existing engines. Update macOS `SCStream` and Qt Audio engines to capture these new sources.

#### Phase 10: Command Palette & Context Menus
- **What is being built:** A global `Ctrl+K` searchable command menu that aggregates all actions across the app (Start Recording, Add Text, Apply Template).
- **Existing Systems Reused:** Centralized application commands.
- **New Components:** `CommandPalette.qml` acting as a fast fuzzy-search overlay over the entire application.

#### Phase 11: Timeline Markers & Chapters
- **What is being built:** The ability to drop colored markers or chapter titles onto the timeline during or after a recording to highlight important moments.
- **Existing Systems Reused:** The existing Timeline QML rendering.
- **New Components:** Extending the Project JSON to store an array of `Marker` objects (timestamp, color, note), and rendering them as visual pips on the `TimelineArea`.

---

### TIER 3 — SHARING & OS INTEGRATION
#### Phase 12: Native Sharing & File System Interface
- **What is being built:** Options to quickly copy, reveal, or share an asset.
- **Existing Systems Reused:** None.
- **New Components:** `IFileSystemIntegration` and `IShareProvider` C++ interfaces with platform-specific implementations (e.g., opening Finder on Mac, Explorer on Windows).

#### Phase 13: Batch Export Queue
- **What is being built:** An export manager that can handle rendering multiple projects sequentially in the background while the UI remains fully responsive.
- **Existing Systems Reused:** `ExportEngine`.
- **New Components:** An `ExportQueueManager` running a bounded background thread, processing a FIFO queue of `.ssa` project paths.

---

### TIER 4 & 5 — ADVANCED & POLISH
#### Phase 14: Redaction Engine
- **What is being built:** Privacy-focused blurring or solid block overlays that can follow on-screen elements to hide sensitive data (like API keys).
- **Existing Systems Reused:** `ExportEngine` compositing pipeline.
- **New Components:** A new `RedactionLayer` that applies Gaussian blur or pixelation over specific X/Y coordinate bounding boxes during the final GPU/FFmpeg render pass.

#### Phase 15: Settings, Privacy & Packaging
- **What is being built:** A unified Preferences window, Privacy controls ensuring local-first execution, and production distribution builds.
- **Existing Systems Reused:** Application configuration logic.
- **New Components:** `SettingsView.qml`, and final CMake adjustments for building `.dmg` (macOS), `Installer` (Windows), and `AppImage` (Linux).

## Open Questions
- Do you want to start immediately with **Phase 1 (System Tray & Global Capture Menu)**, or would you like to tackle the **Library / SQLite Database (Phase 4)** first to serve as the anchor for the application?
- Should we use Qt's native `QSystemTrayIcon` for Phase 1, or build a custom frameless window for the menu bar?

## Verification Plan
For every single phase, the strict verification rule is:
1. Compile the app (`make -C build`).
2. Verify the new feature works natively.
3. Verify that the original Screen Recorder and Editor still work perfectly with zero regressions in performance.
