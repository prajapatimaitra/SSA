# Implementation Plan: Phase 6 (Basic Editor UI)

Now that we have successfully captured and serialized video frames and mouse metadata into a custom project format (which is now saving exactly as requested into the hidden `.recordings/` folder inside your `ssA/` directory!), it's time to build the foundation for our Editor interface.

## Proposed Changes

We will transition the application state and load a distinct Editor view when a recording stops.

### State & QML Navigation

#### [MODIFY] [src/ui/MainWindow.qml](file:///Users/maitraprajapati/Desktop/ssA/src/ui/MainWindow.qml)
We will introduce a Qt Quick `StackView` or `Loader` to act as a router.
- When `ApplicationState` is `IDLE` or `RECORDING`, we display the current recorder UI.
- When the user clicks **Stop Recording**, we will transition the `ApplicationState` to `EDITING`, and the UI will swap to the new `EditorWindow.qml`.

### Editor Interface

#### [NEW] [src/ui/EditorWindow.qml](file:///Users/maitraprajapati/Desktop/ssA/src/ui/EditorWindow.qml)
A modern, dark-themed Qt Quick component representing the Editor.
- **Top Bar**: Shows the Project UUID and basic controls (Export, Back).
- **Preview Area**: A placeholder rectangle where the hardware-accelerated video player (Phase 10) will eventually be embedded.
- **Timeline**: A bottom panel featuring a scrubber slider and timeline tracks.

#### [MODIFY] [src/app/main.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/app/main.cpp)
#### [MODIFY] [src/capture/CaptureController.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/capture/CaptureController.cpp)
We will expose the `ProjectManager` directly to QML or via a new `EditorController` so that the `EditorWindow.qml` can read the newly generated project UUID, mouse click count, and video dimensions to display them in the UI!

### Resource Registration
#### [MODIFY] [src/ui/qml.qrc](file:///Users/maitraprajapati/Desktop/ssA/src/ui/qml.qrc)
Register the new `EditorWindow.qml` file.

## Verification Plan

### Manual Verification
- Launch the application and start a recording.
- Move the mouse around and stop the recording.
- Verify that the UI smoothly transitions from the basic Recorder interface to a brand new Editor interface.
- Verify that the Editor displays the correct UUID and click count from the `.ssa` project bundle you just created!
