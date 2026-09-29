# Phase 11: Timeline Markers & Chapters

Implement the ability to drop colored markers or chapter titles onto the timeline during or after a recording to highlight important moments.

## User Review Required
- Should we add a dedicated "Add Marker" button in the Editor UI, or just rely on a keyboard shortcut (e.g., pressing `M` while playing)? (I plan to add both).
### Ans 
i'll recommend to go for both but dont go with letter M because in many media player M is for mute have some another shortcut
- What should the default marker color be? (I plan to use a bright yellow/orange).
### Ans 
whichever color suits the best


## Proposed Changes

### 1. Data Model
#### [MODIFY] `src/project/ProjectMetadata.h`
- Define a new `Marker` struct:
  ```cpp
  struct Marker {
      uint64_t timestampMs;
      std::string color;
      std::string note;
  };
  ```
- Add `std::vector<Marker> markers;` to the `ProjectMetadata` struct.

#### [MODIFY] `src/project/MetadataSerializer.cpp`
- Update the `serialize` and `deserialize` methods to save/load the `markers` array to/from the project's `.json` metadata file.

### 2. Backend Controller
#### [MODIFY] `src/editor/EditorController.h` & `src/editor/EditorController.cpp`
- Expose a `QVariantList` property `markers` to QML to pass the marker data.
- Add `Q_INVOKABLE void addMarker(qint64 timestampMs, const QString& note, const QString& color);`
- Add `Q_INVOKABLE void removeMarker(int index);`
- Trigger a `markersChanged` signal whenever a marker is added or removed.

### 3. Timeline UI
#### [MODIFY] `src/ui/components/TimelineTrack.qml`
- Add a `Repeater` bound to `editorController.markers`.
- For each marker, draw a small colored triangle (or colored `Rectangle` pip) positioned at `(marker.timestampMs / editorController.totalDuration) * width`.
- Add a hover tooltip (using `ToolTip`) to display the marker's `note`.

#### [MODIFY] `src/ui/EditorView.qml`
- Add an "Add Marker" button in the transport controls (play/pause area).
- Add a keyboard shortcut `M` that triggers `editorController.addMarker(editorController.currentTime, "Chapter", "#f39c12")`.
