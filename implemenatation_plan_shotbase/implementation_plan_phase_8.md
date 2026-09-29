# Phase 8: Project Duplication & Templates

You are absolutely right. I got confused by an older PRD document and mistakenly implemented the "Advanced Timeline" as Phase 7, completely skipping the Shotbase plan's Phase 7 ("Project Import") and jumping off track. 

Since you requested to proceed with Phase 8, we will jump back onto the Shotbase track with **Phase 8: Project Duplication & Templates**.

## Goal Description
We will implement the ability to reuse layouts and non-destructively duplicate projects. Duplicating a screen recording project shouldn't mean duplicating a 2GB video file. We will also introduce a Template Engine that can extract stylistic choices (colors, scales) into reusable presets.

## User Review Required
Please review the proposed approach for deduplicating video files during project duplication.

## Open Questions
> [!IMPORTANT]
> **Video Deduplication Strategy:** To duplicate a project without copying the massive video file, I propose using OS-level **Hard Links**. This means the new `.ssa` project bundle will have a `video.mp4` that points to the exact same bytes on the hard drive as the original. If you delete the original project, the duplicated project's video *will still survive* because hard links protect the underlying data until all references are gone. Does this behavior align with your expectations, or would you prefer the new project's metadata to just contain an absolute path pointing back to the original project folder?

## Proposed Changes

### 1. Project Duplication Engine
#### [MODIFY] `src/project/ProjectManager.h` & `ProjectManager.cpp`
- Add `bool duplicateProject(const QString& sourcePath, const QString& newName)`.
- The function will create a new `.ssa` bundle.
- It will copy the `metadata.json` and generate a new UUID for the duplicated project.
- It will use `std::filesystem::create_hard_link` to link `video.mp4` and `webcam.mp4` into the new bundle, ensuring instant duplication with 0 bytes of extra disk space.

### 2. Template Extraction Engine
#### [NEW] `src/project/TemplateManager.h` & `TemplateManager.cpp`
- A new engine to handle visual presets.
- Add `saveTemplateFromProject(const ProjectMetadata& metadata, const QString& templateName)`. This will extract purely stylistic properties (like `cursorColor`, `cursorScale`, `backgroundColor`) and save them as a JSON file in the application's configuration directory.
- Add `applyTemplateToProject(ProjectMetadata& metadata, const QString& templateName)` to instantly apply a saved look to any project.

### 3. Editor UI Integration
#### [MODIFY] `src/ui/EditorView.qml`
- Add a "Save as Template..." button in the properties panel to extract the current look.
- Add a "Apply Template" dropdown or button to load a saved look.
- Update `EditorController` to expose these endpoints to QML.

## Verification Plan
### Manual Verification
1. Open a recording in the editor, change the background and cursor colors.
2. Click "Save as Template", name it "Dark Mode Preset".
3. Open a brand new recording, click "Apply Template: Dark Mode Preset" and verify the styles update instantly.
4. From the C++ backend, trigger `duplicateProject` on a 1GB recording and verify it duplicates instantly without consuming an additional 1GB of disk space.
