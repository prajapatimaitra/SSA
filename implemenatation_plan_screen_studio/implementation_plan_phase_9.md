# Phase 9: Automatic Camera (Smart Zoom)

This is the signature feature of the app: automatically turning a boring screen recording into a highly dynamic, engaging video.

## Goal
Implement the `SmartZoomAnalyzer` to evaluate the array of recorded mouse click metadata and automatically generate precise, deterministic camera keyframes that follow the action.

## Proposed Changes

### C++ Analyzers
#### [NEW] [SmartZoomAnalyzer.h](file:///Users/maitraprajapati/Desktop/ssA/src/playback/SmartZoomAnalyzer.h) & [SmartZoomAnalyzer.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/playback/SmartZoomAnalyzer.cpp)
- **Class**: `SmartZoomAnalyzer`
- **Method**: `static std::vector<project::CameraKeyframe> generateKeyframes(const project::ProjectMetadata& metadata)`
- **Logic**:
  - We will iterate through the raw `mouseEvents` to locate `Down` (click) actions.
  - For each distinct click, the analyzer will automatically generate a cluster of 4 keyframes:
    1. **Pre-action**: Normal zoom (1.0) ~1 second before the click.
    2. **Action Start**: Smoothly ease into a 2.0x zoom centered perfectly on the click's X/Y coordinates right as the click happens.
    3. **Hold**: Maintain the 2.0x zoom on the target for ~1.5 seconds.
    4. **Release**: Smoothly ease back out to 1.0x normal zoom centered on the screen.

### C++ Integration
#### [MODIFY] [PlaybackEngine.cpp](file:///Users/maitraprajapati/Desktop/ssA/src/editor/PlaybackEngine.cpp)
- Remove the temporary hardcoded test keyframes that I wrote in Phase 8.
- Invoke `SmartZoomAnalyzer::generateKeyframes(metadata)` immediately when a project is loaded, and feed the output dynamically into `m_cameraEngine`.

#### [MODIFY] [CMakeLists.txt](file:///Users/maitraprajapati/Desktop/ssA/src/CMakeLists.txt)
- Add `src/playback/SmartZoomAnalyzer.cpp` to the build pipeline.

## Open Questions

> [!TIP]
> Your blueprint designates "Advanced Grouping" (grouping multiple rapid clicks into a single wide pan rather than erratic bouncing zooms) for **Phase 12**. 
> 
> Therefore, for Phase 9, I will build the fundamental logic: simple, isolated zooming in and out on each individual click interaction. It will look fantastic for sparse clicks, and we will enhance the intelligence in Phase 12.

Do you approve this plan to implement the Smart Zoom engine?
