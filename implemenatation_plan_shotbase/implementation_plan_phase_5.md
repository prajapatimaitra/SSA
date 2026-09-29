# Phase 5: Workspace UI (Tags, Search, Recent)

The goal of this phase is to replace the simple "Start Recording" screen with a full **Workspace Home Screen**. This workspace will display a beautiful, high-performance Grid view of all your cataloged screenshots and video recordings, allowing you to search through them and open them instantly.

## Open Questions
- **Tags Implementation:** For this initial phase, do you want to manually type tags into a text field on each item, or should we just implement the Search bar to filter by filename/date for now, and leave user-defined tags for a later polish phase?
- **Layout Preferences:** I plan to build a visually rich Grid of media "Cards" (similar to modern apps like Figma or Notion). The "Start Recording" button will be moved to a prominent spot in the top header. Does this layout sound good to you?

## Proposed Changes

### Data Layer (C++)
#### [NEW] src/library/LibraryModel.h
#### [NEW] src/library/LibraryModel.cpp
- Create a `QAbstractListModel` that interfaces with the `LibraryManager` (built in Phase 4).
- It will expose roles to QML: `idRole`, `typeRole`, `pathRole`, `timestampRole`, `thumbnailPathRole`, `durationRole`.
- The model will load the data from SQLite and notify the QML UI of any updates (e.g., when a new screenshot is taken).

#### [NEW] src/library/LibraryFilterModel.h
#### [NEW] src/library/LibraryFilterModel.cpp
- Create a `QSortFilterProxyModel` to handle instant text-based search filtering. When you type in the search bar, this model will filter the grid down to matches instantly in C++ memory.

### UI Layer (QML)
#### [NEW] src/ui/WorkspaceView.qml
- A complete redesign of the initial screen.
- **Header:** Features the "Start Recording" button, Camera selector, and a Search Bar.
- **Grid:** A `GridView` displaying all your media.

#### [NEW] src/ui/components/MediaCard.qml
- A reusable UI component for each item in the grid.
- It will display the thumbnail, a Video/Screenshot icon badge, and a beautifully formatted date.
- **Interactions:**
  - Clicking a **Video** will load the project into the Video Editor (`EditorView`).
  - Clicking a **Screenshot** will instantly open it in the `ScreenshotEditor` overlay.

#### [MODIFY] src/ui/MainWindow.qml
- Replace `RecorderView` with `WorkspaceView`.

#### [MODIFY] src/app/main.cpp
- Instantiate `LibraryModel` and `LibraryFilterModel` and register them as QML context properties so the UI can bind to them.

## Verification Plan
### Manual Verification
- Compile the application and ensure it boots into the new Workspace.
- Verify the Grid accurately populates with the existing recordings and screenshots via the SQLite database.
- Verify typing in the search bar instantly filters the Grid.
- Verify clicking a video correctly routes to the Video Editor.
- Verify clicking a screenshot correctly routes to the Screenshot Editor.
