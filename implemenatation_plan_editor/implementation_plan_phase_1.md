# Phase 1 Implementation Plan: Project Bins & Asset Database

## Overview
This plan outlines the architecture and step-by-step implementation for **Phase 1: Project Bins & Asset Database** of the 50-Phase Standalone Non-Linear Editor (NLE). 
Phase 1 establishes a hierarchical project asset database (Bins & Media Items) supporting multi-folder bin structures, media file metadata parsing, SQLite database persistence inside project bundles, and a responsive QML Media Pool panel.

---

## Technical Specifications & Architecture

### 1. Database Schema & Data Models
- **Project Bins (`ProjectBin`)**:
  - `bin_id` (Primary Key UUID/Int)
  - `parent_bin_id` (Nullable Foreign Key for nested folder hierarchy)
  - `name` (String, e.g., "A-Roll", "Audio Stems", "B-Roll/Bites")
  - `color_tag` (String hex code for visual organization)
  - `created_at` (Timestamp)
- **Asset Items (`ProjectAsset`)**:
  - `asset_id` (Primary Key UUID/Int)
  - `bin_id` (Foreign Key -> `ProjectBin`)
  - `name` (String display name)
  - `file_path` (Absolute filesystem path or relative bundle path)
  - `media_type` (Enum: `Video`, `Audio`, `Image`, `TitleSequence`)
  - `duration_ms` (Int64)
  - `width`, `height`, `fps` (Video metadata)
  - `sample_rate`, `channels` (Audio metadata)
  - `thumbnail_path` (Cached preview thumbnail image path)
  - `created_at`, `file_size_bytes`

### 2. C++ Backend Architecture
- **`ssa::project::AssetDatabase`** (`src/project/AssetDatabase.h/.cpp`):
  - Manages SQLite tables inside project database (`project.db` or `ssa_library.db`).
  - Thread-safe operations for:
    - Creating/renaming/deleting bins (with cascade handling for assets & child bins).
    - Importing media files with metadata extraction.
    - Moving assets between bins.
    - Searching and filtering assets.
- **`ssa::project::ProjectBinModel`** (`src/project/ProjectBinModel.h/.cpp`):
  - `QAbstractItemModel` implementation exposing hierarchical bin trees to QML.
  - Exposes roles: `BinIdRole`, `ParentBinIdRole`, `NameRole`, `ColorTagRole`, `ChildCountRole`.
- **`ssa::project::AssetListModel`** (`src/project/AssetListModel.h/.cpp`):
  - `QAbstractListModel` exposing assets contained within a selected bin.
  - Exposes roles: `AssetIdRole`, `BinIdRole`, `NameRole`, `FilePathRole`, `MediaTypeRole`, `DurationRole`, `ResolutionRole`, `ThumbnailPathRole`, `FormattedDurationRole`.

### 3. QML Media Pool UI Component
- **Bin Tree View (`src/ui/components/media/BinTreeView.qml`)**:
  - Sidebar showing collapsible folder tree of project bins.
  - Context menu: "New Bin", "New Sub-Bin", "Rename", "Delete", "Change Color".
- **Asset Grid/List View (`src/ui/components/media/AssetGridView.qml`)**:
  - Main panel showing asset thumbnails, asset name, duration badge, and media type indicator.
  - Drag-and-Drop support for dropping external video/audio files directly into bins.
  - Double-click to preview asset in Source Monitor.

---

## Proposed Changes

### Component 1: C++ Data Models & SQLite Manager

#### [NEW] [`AssetDatabase.h`](file:///Users/maitraprajapati/Desktop/ssA31/src/project/AssetDatabase.h)
#### [NEW] [`AssetDatabase.cpp`](file:///Users/maitraprajapati/Desktop/ssA31/src/project/AssetDatabase.cpp)
- Implements SQLite initialization (`bins` and `project_assets` tables).
- Methods: `createBin()`, `deleteBin()`, `renameBin()`, `importAsset()`, `moveAssetToBin()`, `getAssetsInBin()`.

#### [MODIFY] [`ProjectManager.h`](file:///Users/maitraprajapati/Desktop/ssA31/src/project/ProjectManager.h)
#### [MODIFY] [`ProjectManager.cpp`](file:///Users/maitraprajapati/Desktop/ssA31/src/project/ProjectManager.cpp)
- Integrate `AssetDatabase` lifetime management when opening/creating `.ssa` project bundles.

---

### Component 2: Qt Models for QML Binding

#### [NEW] [`ProjectBinModel.h`](file:///Users/maitraprajapati/Desktop/ssA31/src/project/ProjectBinModel.h)
#### [NEW] [`ProjectBinModel.cpp`](file:///Users/maitraprajapati/Desktop/ssA31/src/project/ProjectBinModel.cpp)
- Hierarchical tree model for QML TreeView / Repeater navigation.

#### [NEW] [`AssetListModel.h`](file:///Users/maitraprajapati/Desktop/ssA31/src/project/AssetListModel.h)
#### [NEW] [`AssetListModel.cpp`](file:///Users/maitraprajapati/Desktop/ssA31/src/project/AssetListModel.cpp)
- List model for displaying items in selected bin with search/filter capabilities.

#### [MODIFY] [`src/CMakeLists.txt`](file:///Users/maitraprajapati/Desktop/ssA31/src/CMakeLists.txt)
- Register new `.cpp` and `.h` files into the CMake build target `SSA`.

---

### Component 3: QML Media Pool & Bin UI

#### [NEW] [`src/ui/components/media/BinTreeView.qml`](file:///Users/maitraprajapati/Desktop/ssA31/src/ui/components/media/BinTreeView.qml)
- Tree representation of project folder bins with folder icon & color tag.

#### [NEW] [`src/ui/components/media/AssetGridView.qml`](file:///Users/maitraprajapati/Desktop/ssA31/src/ui/components/media/AssetGridView.qml)
- Grid & List view switcher for media pool items with thumbnail previews and metadata overlays.

#### [NEW] [`src/ui/components/media/MediaPoolPanel.qml`](file:///Users/maitraprajapati/Desktop/ssA31/src/ui/components/media/MediaPoolPanel.qml)
- Container combining BinTreeView sidebar, search bar, filter tabs, and AssetGridView.

---

## User Review Required

> [!IMPORTANT]
> **Approval Checklist**:
> 1. Do you approve the architecture for **Phase 1: Project Bins & Asset Database**?
> 2. Should imported media be kept at original location or copied into the `.ssa` project bundle's `media/` folder by default?
> 3. Once approved, I will immediately begin implementation of Phase 1 and verify with CMake build and executable checks.

---

## Verification Plan

### Automated Build Verification
- Run `cmake -B build -S .` and `cmake --build build` to ensure clean compilation of C++ models, SQLite integration, and Qt MOC bindings.

### Manual & UI Functional Verification
- Verify creation of root bins and nested sub-bins via backend unit tests/smoke tests.
- Verify file import into bins (video, audio, image) and metadata parsing (duration, resolution).
- Launch `SSA` app and verify Media Pool panel renders bin hierarchy and imported assets.
