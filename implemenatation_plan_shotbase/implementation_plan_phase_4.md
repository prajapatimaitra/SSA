# Phase 4: Local Media Library (SQLite)

The goal of this phase is to build the backend infrastructure to index all of the user's recordings and screenshots. Currently, recordings are saved as `.ssa` bundles and screenshots as `.png` files. To display a fast, responsive gallery in the application (which we will build in Phase 5), we need a local SQLite database to act as a high-speed catalog, avoiding the need to manually scan the file system every time the app opens.

## Open Questions
- **Thumbnail Generation:** For screenshots, the thumbnail is just the image itself. For videos (`.ssa` bundles), we currently don't generate a thumbnail preview image. For now, I will design the database schema to support a `thumbnail_path`, but we may leave it blank for videos until we implement a video frame extractor, unless you'd like me to build a basic thumbnail extractor in this phase?

## Proposed Changes

### Database Layer
#### [NEW] src/library/LibraryManager.h
#### [NEW] src/library/LibraryManager.cpp
- Create a `MediaItem` struct to represent a single row in the database (ID, Type, Path, Timestamp, Duration, Resolution).
- Implement `LibraryManager` class utilizing `QSqlDatabase` with the SQLite driver.
- **Methods:**
  - `initialize()`: Creates the SQLite database file in the app data directory and sets up the `media_items` table if it doesn't exist.
  - `addMediaItem(const MediaItem& item)`: Inserts a new recording or screenshot into the catalog.
  - `removeMediaItem(int id)`: Deletes an item from the database.
  - `getAllItems()`: Fetches all items sorted by timestamp (newest first).
  - `scanExistingFiles(const QString& recordingsDir, const QString& screenshotsDir)`: A utility function to scan the disk for existing `.ssa` and `.png` files and inject them into the database on first boot.

### Integration
#### [MODIFY] src/app/main.cpp
- Instantiate `LibraryManager` globally.
- Call `initialize()` to ensure the database is ready.
- Call `scanExistingFiles()` to catch any recordings you made previously.

#### [MODIFY] src/capture/CaptureController.cpp
- Pass a reference (or callback) to `LibraryManager` into `CaptureController`.
- When a video recording successfully finishes (`stopCapture`), automatically register the new `.ssa` bundle into the SQLite database.
- When a screenshot is captured, automatically register the `.png` file into the SQLite database.

#### [MODIFY] src/CMakeLists.txt
- Add `Sql` to the `find_package(Qt6 COMPONENTS Core Gui Qml Quick Multimedia Sql REQUIRED)` list.
- Link the `Qt6::Sql` module so we can use SQLite natively via Qt.

## Verification Plan
### Automated Verification
- I will run `make` to ensure the Qt SQL module links correctly on macOS.
### Manual Verification
- I will run the application, which will trigger the `scanExistingFiles()` function.
- I will verify the SQLite database file is successfully created on disk.
- I will query the SQLite database via terminal to ensure all your existing recordings and screenshots are perfectly cataloged with correct timestamps.
