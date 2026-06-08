cmake_minimum_required(VERSION 3.16)
project(linuxtimelineviewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Qt6 6.2+ is available in standard package managers on Ubuntu 22.04+
find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets Gui Core Xml)

# Collect all source files
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)

add_executable(LinuxTimelineViewer ${SOURCES} ${HEADERS})

target_link_libraries(LinuxTimelineViewer PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Core
    Qt6::Xml
)

target_include_directories(LinuxTimelineViewer PRIVATE src src/utils)

# Resources
qt_add_resources(APP_RESOURCES resources/icons.qrc)
target_sources(LinuxTimelineViewer PRIVATE ${APP_RESOURCES})

install(TARGETS LinuxTimelineViewer DESTINATION bin)
