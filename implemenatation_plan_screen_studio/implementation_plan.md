# Full Architecture and Implementation Plan: SSA

This document provides a highly detailed, comprehensive architectural breakdown for every phase of the **SSA** application. The goal is to fully specify the data structures, interfaces, and file hierarchy for all 16 phases before any code is written, ensuring a cohesive cross-platform C++ architecture from the ground up.

---

## Architecture Overview

**Application Name:** SSA
**Core Technologies:** C++20, CMake, vcpkg, Qt 6 (QML), FFmpeg (Media Pipeline)
**Platforms:** Windows, macOS, Linux
**Core Principle:** Do not build a video editor that records the screen. Build a recording data system that can render a video. 

---

## PHASE 1 — Project Foundation
Establish the cross-platform CMake build system and Qt 6 foundation.

### Architecture & Info
- **CMake & vcpkg configuration**: Root configuration enforces C++20, configures Qt 6, and sets up platform-specific source sets.
- **Application Entry Point**: Initializes Qt QGuiApplication, registers QML types, and initializes the application state.
- **Application State Machine**: Manages the application lifecycle internally with states such as IDLE, RECORDING, STOPPING, PROCESSING, READY, PLAYING, PAUSED, EXPORTING, ERROR.
- **Structured Logging**: A thread-safe logging mechanism for writing diagnostics to the console and log files without blocking the UI.
- **Main UI Window**: The primary Qt Quick (QML) UI shell that will house the application logic.

---

## PHASE 2 — Platform Detection
Detect OS capabilities, GPUs, and connected displays.

### Architecture & Info
- **Display Manager Interface**: An abstraction for enumerating connected displays, fetching their resolutions, refresh rates, DPI scaling factors, and coordinate origins.
- **GPU Detector Interface**: An abstraction for obtaining hardware GPU information to verify hardware acceleration capabilities.
- **Capability Detector Interface**: An abstraction for querying OS capabilities and required permissions (e.g., Screen Recording and Accessibility permissions).
- **Windows Implementation**: Uses Win32 APIs for displays and DXGI for GPU info.
- **macOS Implementation**: Uses CoreGraphics for displays and Metal for GPU info.
- **Linux Implementation**: Uses Wayland/X11 detection for displays and OpenGL/Vulkan for GPU info.

---

## PHASE 3 — Screen Capture
Implement native hardware-accelerated screen capture on all platforms.

### Architecture & Info
- **Platform Capture Interface**: An abstraction exposing methods to start capture, stop capture, and emit captured video/audio frames via callbacks.
- **Video Frame Representation**: A platform-neutral data model that holds timestamps, dimensions, pixel format, color space, and a pointer to the raw CPU/GPU buffer.
- **Windows Implementation**: Implemented using the modern Windows Graphics Capture API.
- **macOS Implementation**: Implemented using the modern ScreenCaptureKit API.
- **Linux Implementation**: Implemented using xdg-desktop-portal and PipeWire, with an X11 fallback where Wayland/PipeWire is unavailable.

---

## PHASE 4 — Mouse and Click Metadata
Track accurate mouse coordinates and clicks.

### Architecture & Info
- **Input Tracker Interface**: Captures and emits mouse events containing precise timestamps, X/Y coordinates, button states, and event types (move, down, up).
- **Coordinate Mapper**: A robust utility module responsible for converting raw coordinates safely between OS Space, Display Space, Capture Space, Project Space, Camera Space, and Output Space, factoring in DPI and multi-monitor offsets.
- **Windows Implementation**: Relies on low-level Win32 mouse hooks.
- **macOS Implementation**: Relies on CGEventTaps.
- **Linux Implementation**: Relies on Wayland/X11 input event systems.

---

## PHASE 5 — Project System
Non-destructive project saving and loading.

### Architecture & Info
- **Project Data Model**: Represents the in-memory state of the active project, managing references to all media, events, camera keyframes, and appearance settings.
- **Project Serializer**: Handles saving and loading the project to a JSON format (`project.json`), which stores relative paths to raw media (video, audio) and serialized arrays of metadata (clicks, camera keyframes). The raw video and audio are never altered.

---

## PHASE 6 — Playback
Video preview and timeline scrubbing.

### Architecture & Info
- **Media Decoder Interface**: An abstraction for streaming frame caches and asynchronous video decoding, ensuring frames can be retrieved by timestamp efficiently.
- **FFmpeg Decoder**: Hardware-accelerated decoding implementation using FFmpeg.
- **Timeline UI**: Qt Quick components representing the timeline, tracks (video, audio, cursor, camera), and scrubbers for playback.

---

## PHASE 7 — Cursor Engine
Reconstruct the cursor dynamically.

### Architecture & Info
- **Cursor Engine Logic**: Processes raw input metadata through low-pass filtering and spline interpolation (Catmull-Rom) to generate a smooth, lag-free rendered cursor path.
- **Procedural Click Effects**: Generates transient visual animations (like expanding rings and fading opacity) strictly at render-time when a click event is encountered.
- **Render Layer**: The cursor and click effects are treated as a completely independent visual layer overlaid on the video.

---

## PHASE 8 — Manual Camera
Virtual camera engine for custom zoom and panning.

### Architecture & Info
- **Camera Keyframes**: Data points dictating the X/Y center, zoom level, and rotation at a specific timestamp.
- **Camera Engine Logic**: Manages interpolation between camera keyframes using customizable easing mathematical functions (ease-in, ease-out, smoothstep).

---

## PHASE 9 — Automatic Camera (Smart Zoom)
Generate deterministic zooms based on interactions.

### Architecture & Info
- **Smart Zoom Analyzer**: Evaluates the array of click metadata, identifies spatial clusters or dwell times, and calculates a target focus rectangle.
- **Safe Zoom Calculator**: Validates the proposed zoom factor against the source resolution and output resolution to ensure visual quality doesn't degrade beyond a configurable limit.
- **Keyframe Generation**: Automatically generates the corresponding camera keyframes to ease into the target, hold during the interaction, and return.

---

## PHASE 10 — GPU Renderer
Abstract graphics layer to composite the video, cursor, and background.

### Architecture & Info
- **Renderer Abstraction**: A unified, platform-independent rendering pipeline interface.
- **Render Pipeline Steps**: Takes the raw video texture, applies camera crop/zoom transforms, renders the background (solid/gradient/image/blur), applies screen masks (rounded corners, shadow), and composites the cursor and click effects on top.
- **Direct3D Implementation**: GPU backend for Windows.
- **Metal Implementation**: GPU backend for macOS.
- **Vulkan Implementation**: GPU backend for Linux.

---

## PHASE 11 — Export
Hardware-accelerated rendering to standard video formats.

### Architecture & Info
- **Media Encoder Interface**: Abstraction for configuring codec settings (H.264/HEVC, resolution, framerate) and pushing finalized frames to an output file container.
- **Export Manager**: Coordinates the GPU Renderer (rendering offline frames) and the Media Encoder (compressing frames).
- **Windows Implementation**: Media Foundation hardware encoders.
- **macOS Implementation**: VideoToolbox hardware encoders.
- **Linux/Fallback Implementation**: VA-API/NVENC via FFmpeg.

---

## PHASE 12 — Advanced Camera
- **Aspect Ratio Engine**: Dynamically recalculates camera framing constraints and padding when the user switches the output resolution between 16:9, 9:16 (vertical), or 1:1, without touching the underlying raw project data.
- **Advanced Grouping**: Enhances the Smart Zoom heuristics to detect rapid, sequential clicks across a wider area and group them into a single, smooth, continuous pan rather than generating jittery in-and-out zooms.

---

## PHASE 13 — Motion Blur
- **Motion Blur Shader**: A post-processing GPU shader that utilizes temporal sampling of adjacent frames and velocity vectors (calculated from the Cursor Engine and Camera Engine) to apply high-quality motion blur to mouse movements and rapid camera pans.

---

## PHASE 14 — Advanced Editor
- **Advanced UI Components**: Expands the QML interface to support professional video editing features. This includes trimming handles on the timeline, draggable camera keyframes, manual cursor scale/color adjustments, a background customization panel, and export presets (e.g., "YouTube 4K", "TikTok 1080p").

---

## PHASE 15 — Webcam
- **Webcam Capture Interface**: An abstraction for enumerating and capturing frames from connected UVC webcams.
- **Renderer Updates**: The rendering pipeline is updated to support a new top-level compositing layer. This layer takes the webcam texture, applies an alpha mask (e.g., circular or rounded rectangle), adds an outline and shadow, and overlays it onto the final composite frame.

---

## PHASE 16 — AI (Optional/Future)
- **Local ML Processing**: Integration of local machine learning models (e.g., via ONNX Runtime or Whisper.cpp) for post-recording analysis.
- **Features**: Deterministic-fallback audio transcription (generating VTT/SRT subtitles), scene detection, and identifying visually significant UI elements to improve camera grouping heuristics.

---

## User Review Required

> [!IMPORTANT]
> The architectural blueprint for all 16 phases is mapped out above, detailing the architecture, interfaces, and structures that ensure a cross-platform, non-destructive media pipeline. Code blocks have been omitted in favor of structural descriptions.
> 
> As per instructions, development must proceed incrementally. If this comprehensive architectural plan is approved, **we will begin execution strictly on Phase 1 (Project Foundation) and Phase 2 (Platform Detection) only**, ensuring a clean, compiling codebase on all platforms before advancing.
