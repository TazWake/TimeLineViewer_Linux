# LinuxTimelineViewer

LinuxTimelineViewer is a Qt6-based C++ application for visualizing large forensic timeline CSV files in a tabbed GUI. It is designed for efficient handling of multi-GB files and supports both Filesystem and Super timeline formats.

## Features
- Multi-tab viewing (one file per tab)
- Sorting and filtering by column
- On-demand search with column picker
- Row tagging with checkbox for Super timeline format
- Tag persistence (saves to the application data directory as `<filename>.tags`)
- Unsaved changes tracking with exit prompt
- Efficient file access: does not fully load files into RAM
- Column reordering (drag headers) and column hiding (right-click header)
- Works on Ubuntu 22.04 / 24.04 and RHEL 9

## Prerequisites

### C++ compiler (GCC 7+ required for C++17)

Check your version:
```bash
g++ --version
```

**Ubuntu** — install or upgrade:
```bash
# Install build tools if missing
sudo apt install build-essential

# If the version is below 7, install a newer compiler and make it the default
sudo apt install gcc-12 g++-12
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 12
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 12
```

**RHEL / Rocky / AlmaLinux**:
```bash
# Base toolchain
sudo dnf groupinstall "Development Tools"

# If the system GCC is too old, use a GCC Toolset (RHEL 8+)
sudo dnf install gcc-toolset-13
# Activate for the current shell session
scl enable gcc-toolset-13 bash
```

---

### CMake 3.16+

Check your version:
```bash
cmake --version
```

**Ubuntu** — the Kitware APT repository always carries a current release:
```bash
# Add the Kitware signing key and repo
wget -qO- https://apt.kitware.com/keys/kitware-archive-latest.asc \
  | gpg --dearmor \
  | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
```

```bash
# Replace "jammy" with your Ubuntu codename (focal, noble, etc.)
echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] \
  https://apt.kitware.com/ubuntu/ jammy main" \
  | sudo tee /etc/apt/sources.list.d/kitware.list
```

```bash
sudo apt update && sudo apt install cmake
```

**RHEL**:
```bash
sudo dnf install cmake
# If the repo version is below 3.16, install from EPEL:
sudo dnf install epel-release && sudo dnf install cmake
```

---

### Qt 6.2+

Qt 6 is available in the standard package managers for all currently supported Ubuntu and RHEL releases. No custom builds or external installers are required.

**Ubuntu 22.04 / 24.04:**
```bash
sudo apt install qt6-base-dev libgl-dev cmake build-essential
```

`libgl-dev` provides the OpenGL headers that Qt6's GUI module requires. The Qt6 XML module headers are bundled inside `qt6-base-dev` — no separate XML package is needed.

Verify:
```bash
qmake6 --version
```

**RHEL 9 / Rocky / AlmaLinux:**
```bash
# Enable CRB and EPEL if Qt 6 packages are not yet visible
sudo dnf config-manager --set-enabled crb
sudo dnf install epel-release

sudo dnf install \
  qt6-qtbase-devel \
  cmake \
  gcc-c++
```

---

## Build Instructions

```bash
mkdir -p build && cd build && \
cmake .. -DCMAKE_BUILD_TYPE=Release && \
make -j$(nproc)
```

No `CMAKE_PREFIX_PATH` is required — Qt 6 installs to standard system paths that CMake finds automatically.

Output binary: `build/bin/LinuxTimelineViewer`

## Usage

```bash
# Launch and use File → Open to load a timeline
./build/bin/LinuxTimelineViewer

# Enable debug logging (written to ~/.local/share/LinuxTimelineViewer/debug.log)
./build/bin/LinuxTimelineViewer --debug
```

Right-click any column header to hide or show columns. Drag column headers to reorder them.

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
