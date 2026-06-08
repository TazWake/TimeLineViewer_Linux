# LinuxTimelineViewer

LinuxTimelineViewer is a Qt6-based C++ application for visualizing large forensic timeline CSV files in a tabbed GUI. It is designed for efficient handling of multi-GB files and supports both Filesystem and Plaso Super timeline formats.

## Features

- Multi-tab viewing (one file per tab)
- Filtering and search across all columns or a selected column — returns all matching rows at once with live progress
- Column reordering (drag headers) and column hiding (right-click header)
- Row tagging with checkbox for Super timeline format
- Tag persistence (saved to the application data directory as `<filename>.tags`)
- Unsaved changes tracking with exit prompt
- Efficient file access — multi-GB files are never fully loaded into RAM
- JSON and XML auto pretty-printing in the message field of Super timelines
- Works on Ubuntu 22.04 / 24.04 with GNOME desktop (including VMware)

---

## Download (Alpha)

Pre-built binaries for Linux x86-64 are available on the [Releases page](../../releases).

**Requirements:** Ubuntu 22.04 or 24.04 with GNOME desktop. Install the Qt6 runtime if not already present:

```bash
sudo apt install libqt6widgets6 libqt6xml6 libgl1
```

**Run:**
```bash
chmod +x LinuxTimelineViewer-*-linux-x86_64
./LinuxTimelineViewer-*-linux-x86_64
```

Verify the download against the `.sha256` file provided on the Releases page:
```bash
sha256sum -c LinuxTimelineViewer.sha256
```

---

## Building from Source

### Prerequisites

#### C++ compiler (GCC 7+ required for C++17)

```bash
g++ --version
```

**Ubuntu:**
```bash
sudo apt install build-essential
```

**RHEL / Rocky / AlmaLinux:**
```bash
sudo dnf groupinstall "Development Tools"
# If GCC is below 7, use a GCC Toolset (RHEL 8+):
sudo dnf install gcc-toolset-13
scl enable gcc-toolset-13 bash
```

---

#### CMake 3.16+

```bash
cmake --version
```

**Ubuntu** — the Kitware APT repository always carries a current release:
```bash
wget -qO- https://apt.kitware.com/keys/kitware-archive-latest.asc \
  | gpg --dearmor \
  | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

# Replace "jammy" with your Ubuntu codename (focal, noble, etc.)
echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] \
  https://apt.kitware.com/ubuntu/ jammy main" \
  | sudo tee /etc/apt/sources.list.d/kitware.list

sudo apt update && sudo apt install cmake
```

**RHEL:**
```bash
sudo dnf install cmake
# If the repo version is below 3.16, install from EPEL:
sudo dnf install epel-release && sudo dnf install cmake
```

---

#### Qt 6.2+

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

sudo dnf install qt6-qtbase-devel cmake gcc-c++
```

---

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

No `CMAKE_PREFIX_PATH` is required — Qt 6 installs to standard system paths that CMake finds automatically.

Output binary: `build/bin/LinuxTimelineViewer`

---

## Usage

```bash
# Launch and use File → Open to load a timeline
./build/bin/LinuxTimelineViewer

# Enable debug logging (written to ~/.local/share/LinuxTimelineViewer/debug.log)
./build/bin/LinuxTimelineViewer --debug
```

- **Search:** use the column picker and search bar at the top of each tab. All matching rows are shown at once; the status bar shows live progress and a match count.
- **Column reordering:** drag any column header left or right.
- **Column hiding:** right-click any column header for a show/hide checklist.
- **Row tagging:** click the checkbox in the Tag column (Super timelines only). Tags are saved automatically on close or via File → Save Tags.
- **Field detail:** double-click any cell to open the full field content in a resizable popup. JSON and XML are pretty-printed automatically.

---

## Supported File Formats

| Format | Header |
|---|---|
| Filesystem timeline | `Date,Size,Type,Mode,UID,GID,Meta,File Name` |
| Plaso Super timeline | `datetime,timestamp_desc,source,source_long,message,parser,display_name,tag` |

The format is auto-detected from the CSV header row on load.

---

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
└── data/
    └── test_files/
```

## Notes

- See `notes/BRIEF.md` for a full developer guide.
- See `notes/TODO.md` for areas to improve icons and UI polish.
- See `future_plans.md` for planned features.
