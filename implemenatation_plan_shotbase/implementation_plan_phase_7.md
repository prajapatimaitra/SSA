# Phase 7: Project Import & Drag and Drop

## Goal Description
We will add the ability to seamlessly import external media (MP4 videos or images) into the workspace. When a user drags and drops a media file onto the Home Workspace, the application will automatically wrap it in a new `.ssa` project bundle, index it in the SQLite Library, and open it in the editor.

## User Review Required
Please review the proposed drag-and-drop workflow below.

## Open Questions
> [!IMPORTANT]
> **Media Copying Strategy:** When a user drops an external `video.mp4` into the app to create a project, do you want me to *copy* the video file into the `.ssa` bundle (safer, prevents the project from breaking if the user deletes the original file), or create a *hard link* / *symlink* to save disk space? I recommend **copying** for external imports so the `.ssa` project is fully self-contained.
### Ans
i want to go with the copy the video file into the .ssa bundle for safer future approached so that the user can directly access its video 

## Proposed Changes

### 1. Project Generation Engine
#### [MODIFY] `src/project/ProjectManager.h` & `ProjectManager.cpp`
- Add `bool createProjectFromExternalMedia(const QString& mediaPath)`.
- This function will:
  1. Validate the external file (ensure it's a valid MP4 or PNG/JPG).
  2. Create a new `.ssa` bundle directory in the standard recordings path.
  3. Copy the external media file into the bundle as `video.mp4` (or `video.png` if it's an image).
  4. Generate a fresh `metadata.json` tailored for the external media (extracting its dimensions using `FFmpegDecoder` or `QImage`).
  5. Add the new project to the SQLite `LibraryManager`.

### 2. UI Drop Area Integration
#### [MODIFY] `src/ui/WorkspaceView.qml`
- Wrap the main library Grid/List in a `DropArea`.
- Listen for `onDropped` events. When a user drops a file (e.g., `file:///Users/name/Desktop/video.mp4`), extract the file path.
- Trigger `projectManager.createProjectFromExternalMedia(path)`.
- If successful, immediately route the user into the `EditorView` with the newly created project.

## Verification Plan
### Manual Verification
1. Launch the application and sit on the Home Workspace.
2. Drag and drop an external `.mp4` video from Finder/Explorer directly onto the app window.
3. Verify that the app instantly creates a new `.ssa` project and opens the editor.
4. Verify the new project appears in the Library view upon returning.
