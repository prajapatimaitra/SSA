# Phase 6: Auto-Save & Crash Recovery

The goal of this phase is to implement a robust background protection system. When you are editing a video (adjusting zooms, modifying subtitles), the app will automatically save your progress to a safe `.autosave` file. If the application or your OS crashes, you will not lose your editing work.

## Open Questions
- **Recovery Behavior:** When the app detects an orphaned `.autosave` file (meaning the app crashed before a manual save occurred), should it silently and automatically recover your work the next time you open the project, or would you prefer a QML popup asking "Crash detected. Do you want to recover your previous session?" For this initial phase, I recommend **silent auto-recovery** to keep the user experience seamless and the implementation simple. Do you agree?
### Answer
i want seamless exprience but what if the user dont want to load the previous file but still we can go with the plan of seamless experience and then if user dont want it he/she can always go back to start new recording 
i'll go with silent auto-recovery to keep the user experience seamless and the implementation simple


## Proposed Changes

### Data Layer (C++)
#### [NEW] src/editor/AutoSaveManager.h
#### [NEW] src/editor/AutoSaveManager.cpp
- Create a new class that utilizes a background `QTimer`.
- It will hold a reference to the active `ProjectManager` and `EditorController`.
- **Logic:** Every 30 seconds (configurable), it will ask the `EditorController` for the latest metadata state and write it to `metadata.json.autosave` inside the `.ssa` bundle.
- **Commit:** When the user explicitly saves their project (or cleanly exits the editor), the `.autosave` file is promoted to become the primary `metadata.json`.

#### [MODIFY] src/project/ProjectManager.cpp
- Update `loadMetadata()` to first check for the existence of a `metadata.json.autosave` file.
- If the autosave file exists and is newer than `metadata.json`, it will load the autosave file instead, effectively recovering the crashed session.

### Integration
#### [MODIFY] src/editor/EditorController.cpp
- Add methods to expose the current editing state (e.g., `getCurrentMetadata()`) so the `AutoSaveManager` can serialize it.
- Integrate the `AutoSaveManager` lifecycle (start the timer when a project loads, stop and commit when the project closes).

#### [MODIFY] src/CMakeLists.txt
- Add `AutoSaveManager.cpp` and `AutoSaveManager.h` to the build sources.

## Verification Plan
### Manual Verification
- I will open a project in the Editor and simulate making changes.
- I will verify that `metadata.json.autosave` is successfully created in the background.
- I will simulate a "crash" by force-killing the application (`pkill SSA`).
- I will reopen the application, open the project, and verify that the autosaved changes were perfectly recovered despite the crash.
