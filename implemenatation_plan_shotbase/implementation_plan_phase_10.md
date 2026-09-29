# Phase 10: Command Palette & Context Menus

Implement a global `Ctrl+K` / `Cmd+K` searchable command menu that aggregates actions across the app, and introduce context menus for media items.

## Proposed Changes

### 1. Command Palette UI
#### [NEW] `src/ui/components/CommandPalette.qml`
- Create a `Popup` component styled like a spotlight search bar.
- Include a `TextField` for typing commands.
- Include a `ListView` displaying filtered commands.
- Define a list of global commands (e.g., "Start Recording", "Take Screenshot", "Go to Workspace", "Export Video").
- Upon selecting a command, execute its associated callback and close the palette.

#### [MODIFY] `src/ui/qml.qrc`
- Register `components/CommandPalette.qml`.

### 2. Global Shortcut Integration
#### [MODIFY] `src/ui/MainWindow.qml`
- Instantiate `CommandPalette` as a child of `MainWindow` with a high `z` index.
- Add a `Shortcut` component listening for `Ctrl+K` (and `Cmd+K` on Mac) that toggles the visibility of the `CommandPalette` and focuses the search input.

### 3. Context Menus
#### [MODIFY] `src/ui/components/MediaCard.qml`
- Add a `MouseArea` configured to accept `Qt.RightButton`.
- Create a `Menu` (Context Menu) with actions such as "Open", "Delete", and "Reveal in Finder".
- Integrate these actions with the `LibraryManager` (for deletion) and `QDesktopServices` / platform APIs (for revealing in Finder).
