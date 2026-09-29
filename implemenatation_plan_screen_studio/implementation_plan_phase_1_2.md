# SSA Architecture: Phase 1 & 2 Completed State

## Core Principle
The application is built around strict cross-platform boundaries. No platform-specific APIs (Win32, CoreGraphics, X11) leak into the core engine. The core engine only interacts with abstract C++ interfaces.

## Directory Structure
```
SSA/
├── CMakeLists.txt              # Root CMake configuration (C++20, Qt6)
├── vcpkg.json                  # Dependency management
└── src/
    ├── CMakeLists.txt          # Source compilation rules
    ├── app/
    │   └── main.cpp            # Entry point; wires platform factories
    ├── core/
    │   ├── logging/
    │   │   └── Logger.h/cpp    # Thread-safe structured logging
    │   └── state/
    │       └── ApplicationState.h/cpp # QObject state machine
    ├── platform/
    │   ├── PlatformFactory.h   # Abstract instantiation factory
    │   ├── interfaces/
    │   │   ├── ICapabilityDetector.h
    │   │   ├── IDisplayManager.h
    │   │   └── IGpuDetector.h
    │   ├── macos/
    │   │   ├── MacCapabilityDetector.mm
    │   │   ├── MacDisplayManager.mm
    │   │   └── MacGpuDetector.mm
    │   ├── windows/            # (Stubs)
    │   └── linux/              # (Stubs)
    └── ui/
        ├── MainWindow.qml      # Qt Quick Main Window
        └── qml.qrc             # Qt Resource file
```

## How it works
1. `main.cpp` calls `createDisplayManager()`, `createGpuDetector()`, etc. from `PlatformFactory.h`.
2. Depending on the OS, CMake compiles the correct `.cpp` or `.mm` files inside `src/platform/`.
3. The platform files implement the factory functions and return `std::unique_ptr`s of the platform-specific classes.
4. The core engine queries these interfaces and passes standard data (like `std::vector<DisplayInfo>`) to the Qt QML UI.
