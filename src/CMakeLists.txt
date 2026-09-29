set(CMAKE_INCLUDE_CURRENT_DIR ON)

# QML Resources
set(QML_FILES
    ui/main.qml
    ui/EditorView.qml
)

# Add source files
set(SOURCES
    app/main.cpp
    core/logging/Logger.cpp
    core/state/ApplicationState.cpp
    capture/CaptureController.cpp
    capture/WebcamRecorder.cpp
    ai/AIEngine.cpp
    ai/MockTranscriptionModel.cpp
    core/coordinates/CoordinateMapper.cpp
    project/ProjectManager.cpp
    project/PresetManager.cpp
    project/MetadataSerializer.cpp
    project/AssetDatabase.cpp
    project/ProjectBinModel.cpp
    project/AssetListModel.cpp
    project/ProjectBinManager.cpp
    editor/EditorController.cpp
    editor/PlaybackEngine.cpp
    editor/AutoSaveManager.cpp
    editor/AudioWaveformExtractor.cpp
    playback/CameraEngine.cpp
    playback/SmartZoomAnalyzer.cpp
    export/ExportEngine.cpp
    export/ExportQueueManager.cpp
    workspace/SystemTrayController.cpp
    workspace/ShortcutManager.cpp
    workspace/ClipboardManager.cpp
    workspace/NativeIntegrationManager.cpp
    library/LibraryManager.cpp
    library/LibraryModel.cpp
    library/LibraryFilterModel.cpp
    auth/AuthManager.cpp
    auth/UserQuotaManager.cpp
    auth/UserManager.cpp
)

# Add headers (for IDEs)
set(HEADERS
    core/logging/Logger.h
    core/state/ApplicationState.h
    capture/CaptureController.h
    core/coordinates/CoordinateMapper.h
    input/MouseEvent.h
    input/interfaces/IInputTracker.h
    project/ProjectMetadata.h
    project/ProjectManager.h
    project/PresetManager.h
    project/MetadataSerializer.h
    project/AssetDatabase.h
    project/ProjectBinModel.h
    project/AssetListModel.h
    project/ProjectBinManager.h
    editor/EditorController.h
    editor/PlaybackEngine.h
    export/ExportEngine.h
    export/ExportQueueManager.h
    workspace/SystemTrayController.h
    workspace/ShortcutManager.h
    workspace/ClipboardManager.h
    workspace/NativeIntegrationManager.h
    workspace/interfaces/IGlobalShortcutProvider.h
    workspace/interfaces/INativeIntegration.h
    capture/interfaces/IScreenCapturer.h
    library/LibraryManager.h
    library/LibraryModel.h
    library/LibraryFilterModel.h
    auth/AuthManager.h
    auth/UserQuotaManager.h
    auth/UserManager.h
)

# Platform specific sources (stubs for Phase 2)
if (APPLE)
    list(APPEND SOURCES 
        platform/macos/MacDisplayManager.mm
        platform/macos/MacGpuDetector.mm
        platform/macos/MacCapabilityDetector.mm
        capture/macos/MacScreenCapture.mm
        input/macos/MacInputTracker.mm
        export/macos/MacMediaEncoder.mm
        workspace/macos/MacShortcutProvider.mm
        workspace/macos/MacNativeIntegration.mm
        capture/macos/MacScreenCapturer.mm
    )
    list(APPEND HEADERS
        platform/interfaces/IDisplayManager.h
        platform/interfaces/IGpuDetector.h
        platform/interfaces/ICapabilityDetector.h
        capture/interfaces/IPlatformCapture.h
        media/VideoFrame.h
        workspace/macos/MacShortcutProvider.h
        workspace/macos/MacNativeIntegration.h
        capture/macos/MacScreenCapturer.h
    )
    find_library(COREGRAPHICS_LIBRARY CoreGraphics)
    find_library(METAL_LIBRARY Metal)
    find_library(SCREENCAPTUREKIT_LIBRARY ScreenCaptureKit)
    find_library(COREMEDIA_LIBRARY CoreMedia)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(AVFOUNDATION_LIBRARY AVFoundation)
    set(PLATFORM_LIBS ${COREGRAPHICS_LIBRARY} ${METAL_LIBRARY} ${SCREENCAPTUREKIT_LIBRARY} ${COREMEDIA_LIBRARY} ${COREVIDEO_LIBRARY} ${AVFOUNDATION_LIBRARY})
elseif (WIN32)
    list(APPEND SOURCES
        platform/windows/WindowsDisplayManager.cpp
        platform/windows/WindowsGpuDetector.cpp
        platform/windows/WindowsCapabilityDetector.cpp
        capture/windows/WindowsGraphicsCapture.cpp
        input/windows/WindowsInputTracker.cpp
    )
    list(APPEND HEADERS
        platform/interfaces/IDisplayManager.h
        platform/interfaces/IGpuDetector.h
        platform/interfaces/ICapabilityDetector.h
        capture/interfaces/IPlatformCapture.h
        media/VideoFrame.h
    )
    set(PLATFORM_LIBS dxgi)
elseif (UNIX AND NOT APPLE)
    list(APPEND SOURCES
        platform/linux/LinuxDisplayManager.cpp
        platform/linux/LinuxGpuDetector.cpp
        platform/linux/LinuxCapabilityDetector.cpp
        capture/linux/PipeWireCapture.cpp
        input/linux/LinuxInputTracker.cpp
    )
    list(APPEND HEADERS
        platform/interfaces/IDisplayManager.h
        platform/interfaces/IGpuDetector.h
        platform/interfaces/ICapabilityDetector.h
        capture/interfaces/IPlatformCapture.h
        media/VideoFrame.h
    )
    set(PLATFORM_LIBS X11 GL)
endif()

# QML Resources
qt6_add_resources(RESOURCES ui/qml.qrc)

# Find fmt via vcpkg
find_package(fmt CONFIG REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2 Multimedia Sql Svg Network)

# Executable
add_executable(SSA ${SOURCES} ${HEADERS} ${RESOURCES})

# Target properties
set_target_properties(SSA PROPERTIES
    MACOSX_BUNDLE ON
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
)

# Link libraries
target_link_libraries(SSA PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    Qt6::Qml 
    Qt6::Quick 
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Sql
    Qt6::Svg
    Qt6::Network
    fmt::fmt
    ${PLATFORM_LIBS}
)

if (APPLE)
    target_link_libraries(SSA PRIVATE
        "-framework Foundation"
        "-framework ScreenCaptureKit"
        "-framework CoreMedia"
        "-framework AVFoundation"
        "-framework ApplicationServices"
        "-framework CoreVideo"
    )
endif()

# Include directories
target_include_directories(SSA PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Phase 1 Test Binary
add_executable(test_phase1 
    /Users/maitraprajapati/.gemini/antigravity-ide/brain/25e5097a-07e7-457b-b0bb-ea5a5f88ef25/scratch/test_phase1.cpp
    core/logging/Logger.cpp
    project/AssetDatabase.cpp
    project/ProjectBinModel.cpp
    project/AssetListModel.cpp
    project/ProjectBinManager.cpp
)
target_link_libraries(test_phase1 PRIVATE Qt6::Core Qt6::Gui Qt6::Sql fmt::fmt)
target_include_directories(test_phase1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Copy wallpapers and cursors folders to executable output directory
add_custom_command(TARGET SSA POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/wallpapers $<TARGET_FILE_DIR:SSA>/wallpapers
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/cursors $<TARGET_FILE_DIR:SSA>/cursors
)
