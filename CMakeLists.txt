cmake_minimum_required(VERSION 3.20)
project(SSA VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable automatic MOC, UIC, and RCC for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 COMPONENTS Core Gui Qml Quick Svg Network REQUIRED)

# Add subdirectory for source code
add_subdirectory(src)
