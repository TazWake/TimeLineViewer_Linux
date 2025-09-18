# LinuxTimelineViewer

LinuxTimelineViewer is a Qt5.15.13-based C++ application for visualizing large forensic timeline CSV files in a tabbed GUI. It is designed for efficient handling of multi-GB files and supports both Filesystem and Super timeline formats.

## Features
- Multi-tab viewing (one file per tab)
- Sorting and filtering by column
- On-demand search with column picker
- Row tagging with checkbox for Super timeline format
- Tag persistence (saves to .tags file alongside original)
- Unsaved changes tracking with exit prompt
- Efficient file access: does not fully load files into RAM
- Statically compiled (no runtime Qt dependencies)
- Works on Ubuntu and RHEL

## Build Instructions

**Requirements:**
- C++17
- CMake 3.16+
- Qt 5.15.13 (built statically)

**Build:**
```bash
# Clone or create project folder
mkdir -p ~/Projects/linuxtimelineviewer && cd ~/Projects/linuxtimelineviewer

# Create a build folder
mkdir build && cd build

# Set static Qt if needed
export CMAKE_PREFIX_PATH=/opt/Qt5.15.13-static/lib/cmake

# Run cmake with static flags
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON

# Build
make -j$(nproc)

# Output binary will be in: build/bin/timeline-explorer
```

## Usage

To open a timeline file:
```bash
./build/bin/timeline-explorer data/test_files/FILESYSTEM.txt
```

### Currently supported method:

`./build/bin/timeline-explorer` 

then file open menu.



Or use the File → Open menu in the application.

## Project Structure
```
linuxtimelineviewer/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── AppWindow.h/.cpp
│   ├── TimelineTab.h/.cpp
│   ├── TimelineModel.h/.cpp
│   ├── TimelineParser.h/.cpp
│   ├── FilterBar.h/.cpp
│   └── utils/
│       ├── JsonXmlFormatter.h/.cpp
│       └── FileUtils.h/.cpp
├── resources/
│   ├── icons.qrc
│   └── appicon.png
├── data/
│   └── test_files/
└── build/
```

## Test Files
- Place example files in `data/test_files/`:
  - `FILESYSTEM.txt` (filesystem timeline)
  - `SUPER.txt` (plaso supertimeline)

## Notes
- See `notes/BRIEF.md` for a full developer guide.
- See `notes/TODO.md` for areas to improve icons and UI polish.
