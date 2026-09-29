# Product Requirement Document (PRD) - Feature Specifications

## 1. System Tray & Menu Bar Launcher
- **Persistent Quick-Access Launcher**: Resides in the operating system status bar / menu bar for instant access without opening the main window.
- **Capture Menu**: Dropdown interface enabling one-click triggers for full display recording, application window capture, custom region capture, or screenshot creation.
- **Recording Status Display**: Real-time visual recording indicators including a pulsing status light and live elapsed time counter.

---

## 2. Global System Hotkeys
- **Customizable Shortcuts**: System-wide keyboard shortcuts for key user actions:
  - Start / Stop Recording
  - Pause / Resume Recording
  - Take Region Screenshot
  - Toggle Webcam Overlay
  - Toggle Audio Recording Sources
- **Background Event Listening**: Responds to defined hotkeys even when the main application is minimized or running in the background.

---

## 3. Command Palette (`Cmd+K` / `Ctrl+K`)
- **Fuzzy-Search Overlay**: Searchable command overlay accessible anywhere within the application.
- **Instant Keyboard Navigation**: Direct execution of actions, workspace tab switching, template application, annotation activation, and export triggers via keyboard search.

---

## 4. Native System Integration & Clipboard Sharing
- **Direct Clipboard Copy**: Copy rendered media, screenshots, or exported videos directly to the system clipboard for immediate pasting into external apps.
- **File Manager Integration**: Single-click "Show in File Manager" action to locate project bundles or exported files in the native file system browser.

---

## 5. Screen Region, Display & Window Capture
- **Multi-Display Capture**: Enumerate and select any connected physical monitor for screen capture.
- **Window Lock Capture**: Select individual application windows to restrict recording boundaries exclusively to that window.
- **Interactive Region Capture**: Draw custom screen recording bounding boxes with draggable handles, dimension previews, and aspect ratio constraint locks (16:9, 1:1, 4:3).

---

## 6. Recording Quality & Framerate Controls
- **Framerate Selection**: Toggle between standard 30 FPS and ultra-smooth 60 FPS recording modes.
- **Target Resolution Profiles**: Selectable capture resolution settings to balance file size and visual fidelity.

---

## 7. Webcam Overlay & Avatar Studio
- **Dual Stream Recording**: Captures webcam video synchronously alongside screen recording.
- **Avatar Shapes**: Switch avatar overlay shapes between Circular, Rounded Rectangle, Square, or Borderless modes.
- **Custom Styling**: Adjust stroke border color, thickness, and drop shadow opacity/blur.
- **Interactive Positioning & Scaling**: Drag webcam overlay anywhere on canvas and scale its size dynamically.
- **Timeline Visibility**: Toggle webcam overlay visibility on/off at specific timestamps along the video timeline.

---

## 8. Multi-Source Audio Capture & Dual-Stream Recording
- **Simultaneous Audio Capture**: Record user microphone voiceover and desktop system audio concurrently.
- **Dual Independent Tracks**: Stores microphone and system audio as separate audio channels within the project for independent post-recording volume balancing, muting, or trimming.

---

## 9. Procedural Cursor Trajectory Smoothing
- **Jitter Removal**: Automatically filters out erratic mouse hand movements and micro-stutters.
- **Spline Path Reconstruction**: Re-renders mouse paths along smooth curvature splines for fluid cursor movement.

---

## 10. Dynamic Visual Click Effects
- **Procedural Click Visuals**: Renders visual animations at the exact location and timestamp of mouse clicks.
- **Customizable Click Styles**: Select expanding rings, ripple pulses, or spotlight glows with custom colors, radius, fade duration, and distinct visuals for left, right, and double clicks.

---

## 11. Post-Recording Cursor Customization
- **Dynamic Cursor Scaling**: Scale cursor size from 1.0x (native) up to 4.0x.
- **Cursor Graphic Variants**: Swap between system cursor variants, high-contrast themes, or custom cursor graphic assets.
- **Visibility Modes**: Choose between always visible, hidden, or visible only during click events.

---

## 12. Automatic Smart Zoom (Interaction-Driven)
- **Interaction-Based Framing**: Analyzes click clusters and hover dwell times to automatically determine focus areas on active UI elements.
- **Automated Smooth Keyframing**: Automatically generates smooth zoom-in, hold, and zoom-out camera transitions centered around key user interactions.
- **Quality Guard Limit**: Enforces a configurable maximum zoom limit to prevent visual pixelation on low-resolution screen areas.

---

## 13. Smart Continuous Panning & Click Grouping
- **Sequential Click Grouping**: Automatically groups rapid, sequential mouse clicks across neighboring screen areas into a single smooth continuous camera pan, avoiding rapid zoom cycles.

---

## 14. Manual Camera Controls & Keyframe Timeline
- **Manual Keyframe Placement**: Add custom camera keyframes along the video timeline.
- **Camera Adjustments**: Controls for X/Y focal point, zoom factor (1.0x to 5.0x), and camera tilt/rotation.
- **Easing Motion Curves**: Easing options including Ease-In-Out, Linear, Smoothstep, and Bounce/Elastic curves.

---

## 15. Non-Destructive Multi-Track Timeline Editor
- **Multi-Track Track Layout**: Visual timeline channels for Screen Capture, Webcam, Microphone Audio, System Audio, Cursor Events, Camera Keyframes, and Subtitles.
- **Non-Destructive Trimming**: Trim start/end points, split video clips into segments, scrub, and step frame-by-frame without altering raw source media.

---

## 16. Timeline Markers, Notes & Chapters
- **Color-Coded Markers**: Drop visual markers onto the timeline to highlight important beats.
- **Notes & Chaptering**: Attach text notes and chapter titles to markers for video structuring and export chapter generation.

---

## 17. Multi-Track Audio Waveform Mixer
- **High-Resolution Waveforms**: Visual audio waveform rendering for microphone and system audio channels.
- **Track Controls**: Independent volume gain sliders, mute toggles, audio ducking, and fade-in/fade-out curve controls.

---

## 18. Framed Studio Canvas & Adjustable Padding
- **Canvas Framing**: Surrounds screen recording content with an outer background canvas for framed presentation.
- **Independent Padding Controls**: Adjustable vertical and horizontal margin padding controls.

---

## 19. Screen Frame Styling (Rounded Corners & Drop Shadows)
- **Rounded Corners**: Adjustable corner radius for the recorded screen container.
- **3D Drop Shadow**: Customizable drop shadow effect under the screen container with controls for shadow blur, offset, spread, and opacity.
- **Border Stroke**: Optional outer frame stroke outline with custom color and opacity.

---

## 20. Canvas Background Styling
- **Solid Colors**: Custom background color selection.
- **Curated Gradients**: Collection of multi-stop linear and radial gradient presets.
- **Custom Images**: Import custom wallpaper or branding background images.
- **Blurred Screen Atmosphere**: Generates a blurred version of the recording itself as a canvas background.

---

## 21. GPU Temporal Motion Blur
- **Motion Blur Shader**: Temporal motion blur post-processing applied to rapid mouse sweeps and camera pans.

---

## 22. Screenshot Capture & Vector Annotation Editor
- **Instant Annotation Launch**: Capturing a screenshot opens an interactive annotation editor window.
- **Vector Annotation Tools**: Straight/curved arrows, geometric shapes, highlight boxes, spotlight focus dimming, text callouts, and numbered step counter badges (1, 2, 3...).

---

## 23. Privacy Redaction Engine (Blur & Pixelate)
- **Interactive Redaction Tool**: Draw blur or pixelate boxes over sensitive screen elements (passwords, credentials, API keys) to obscure them on static screenshots and video timelines.

---

## 24. Local Media Library & Catalog
- **Central Workspace Catalog**: Hub listing all past recordings, projects, and screenshots in a grid or list view.
- **Search, Tags & Collections**: Full-text search, user-created tags, color categories, and smart collections.

---

## 25. Non-Destructive Project Bundles & Duplication
- **Metadata Project Bundles**: `.ssa` project files store references to media assets non-destructively.
- **Instant Project Duplication**: Duplicate existing projects to test alternate edits or styles without copying raw video source files.

---

## 26. Auto-Save & Crash Protection
- **Continuous Auto-Save**: Real-time saving of timeline edits, camera keyframes, cursor adjustments, and style settings.
- **Session Recovery**: Automatic detection of abnormal app shutdowns with prompt to restore un-saved workspace state upon launch.

---

## 27. Style Templates & Design Profiles
- **Custom Style Templates**: Save complete canvas styling configurations (background gradient, padding, shadows, cursor size, webcam mask) as reusable templates.
- **One-Click Application**: Apply saved templates across projects for consistent design presentation.

---

## 28. Dynamic Aspect Ratio Reframer
- **Multi-Platform Reframing**: Reframes landscape recordings for 16:9 Landscape (YouTube), 9:16 Portrait (TikTok/Reels/Shorts), and 1:1 Square (LinkedIn).
- **Auto Camera Re-Centering**: Camera automatically maintains focus on active mouse actions when switching aspect ratios.

---

## 29. Export Quality & Format Controls
- **Format Presets**: Export profiles including YouTube 4K, Social Media 1080p, Web 720p, and Animated GIF.
- **Custom Encoder Controls**: Manual control over export resolution (up to 4K), framerate (24, 30, 60 FPS), video/audio bitrates, and keyframe intervals.

---

## 30. Background Batch Export Queue
- **Sequential Render Queue**: Add multiple projects to a background export queue.
- **Non-Blocking Execution**: Background render execution allows users to continue editing or browsing without UI lag.

---

## 31. Local Speech-to-Text Subtitle Generator
- **Offline Transcription**: Offline speech-to-text processing for automatic closed captioning.

---

## 32. On-Screen Subtitle Styling & SRT/VTT Export
- **On-Video Caption Styling**: Render styled captions directly onto video canvas with custom fonts, colors, and background highlight pills.
- **Subtitle Export**: Export captions as standard `.vtt` or `.srt` files.
