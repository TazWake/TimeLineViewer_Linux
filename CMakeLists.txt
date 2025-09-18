cmake_minimum_required(VERSION 3.16)
project(linuxtimelineviewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC for QObject subclasses
set(CMAKE_AUTOMOC ON)

# Enable static Qt build
set(BUILD_STATIC ON CACHE BOOL "Build statically against Qt")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find Qt5 (add Xml module)
find_package(Qt5 5.15.13 REQUIRED COMPONENTS Widgets Gui Core Xml)

# Collect all source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
)
file(GLOB_RECURSE HEADERS
    src/*.h
)

# Add executable (renamed)
add_executable(LinuxTimelineViewer ${SOURCES} ${HEADERS})

# Link Qt5 libraries (add Qt5::Xml)
if(BUILD_STATIC)
    set_target_properties(LinuxTimelineViewer PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(LinuxTimelineViewer PRIVATE Qt5::Widgets Qt5::Gui Qt5::Core Qt5::Xml)
else()
    target_link_libraries(LinuxTimelineViewer PRIVATE Qt5::Widgets Qt5::Gui Qt5::Core Qt5::Xml)
endif()

# Include directories
target_include_directories(LinuxTimelineViewer PRIVATE src src/utils)

# Add resources (icons, etc.)
qt5_add_resources(APP_RESOURCES resources/icons.qrc)
target_sources(LinuxTimelineViewer PRIVATE ${APP_RESOURCES})

# Install rules (optional)
install(TARGETS LinuxTimelineViewer DESTINATION bin)

# Notes:
# - To build statically, ensure you have a static Qt5.15.13 build and set CMAKE_PREFIX_PATH accordingly.
# - Out-of-source builds are supported (use a separate build/ directory). 