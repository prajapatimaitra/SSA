# Implementation Plan: Phase 5 (Project System)

This plan outlines the architecture for our non-destructive `.ssa` project format. This system will allow us to serialize the mouse metadata we captured in Phase 4 and associate it with our video stream for the editor.

## User Review Required

> [!IMPORTANT]
> For this phase, I propose using a **Directory Bundle** format (e.g., `MyRecording.ssa/`). 
> On macOS, directories ending in specific extensions can be treated as single files by Finder (Package format). This is highly efficient because we don't have to compress/uncompress gigabytes of video into a `.zip` file every time we save.
> Inside the bundle, we will have:
> - `metadata.json`: Contains project dimensions, hardware metadata, and an array of all serialized `MouseEvents`.
> - `video_stream.raw`: (Placeholder for Phase 10 video encoding).
> 
> We will use Qt's native `QJsonDocument` and `QJsonObject` to handle the JSON serialization to avoid adding heavy third-party dependencies like `nlohmann/json` unless required. Is this approach approved?

## Proposed Changes

We will build the `ProjectManager` and serialization logic.

### Core Architecture

#### [NEW] [src/project/ProjectMetadata.h](file:///Users/maitraprajapati/Desktop/ssA/src/project/ProjectMetadata.h)
Defines the `ProjectMetadata` struct, containing:
- Project UUID
- Creation timestamp
- Video Width/Height
- `std::vector<input::MouseEvent> mouseEvents`

#### [NEW] [src/project/ProjectManager.h](file:///Users/maitraprajapati/Desktop/ssA/src/project/ProjectManager.h)
#### [NEW] [src/project/ProjectManager.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/project/ProjectManager.cpp)
A core service responsible for creating, saving, and loading `.ssa` project bundles.
- `bool createNewProject(const std::string& path);`
- `bool saveMetadata(const ProjectMetadata& metadata);`
- `std::optional<ProjectMetadata> loadMetadata();`

#### [NEW] [src/project/MetadataSerializer.h](file:///Users/maitraprajapati/Desktop/ssA/src/project/MetadataSerializer.h)
#### [NEW] [src/project/MetadataSerializer.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/project/MetadataSerializer.cpp)
Uses `QJsonObject`, `QJsonArray`, and `QJsonDocument` to convert the `ProjectMetadata` struct (including all `MouseEvent`s) into a formatted JSON string, and vice versa.

### Integration

#### [MODIFY] [src/capture/CaptureController.h](file:///Users/maitraprajapati/Desktop/ssA/src/capture/CaptureController.h)
#### [MODIFY] [src/capture/CaptureController.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/capture/CaptureController.cpp)
- Update the controller to accumulate `MouseEvent`s in a `std::vector` while recording.
- When `stopCapture()` is called, generate a temporary project bundle in the OS's temp directory and write the events to `metadata.json`.

#### [MODIFY] [src/CMakeLists.txt](file:///Users/maitraprajapati/Desktop/ssA/src/CMakeLists.txt)
- Include the `src/project/` directory in the build.

## Verification Plan

### Automated Tests
- Build system verification to ensure Qt JSON libraries compile correctly across platforms.

### Manual Verification
- Launch the application, click "Start Recording", click the mouse a few times, and click "Stop Recording".
- Verify that a `metadata.json` file is successfully created in the application's local temp directory and inspect its contents to ensure the mouse clicks are perfectly serialized.
