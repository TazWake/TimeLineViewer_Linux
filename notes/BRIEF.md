# 🗂 Linux Timeline Viewer – Developer Build & Implementation Guide

## 📌 Project Overview

**LinuxTimelineViewer** is a Qt-based C++ application for visualizing large forensic timeline CSV files in a tabbed GUI.

**Features:**
- Supports Filesystem timelines and Super timelines (CSV format)
- Multi-tab viewing (one file per tab)
- Sorting and filtering by column
- On-demand search
- Pretty-printing of embedded XML/JSON within message fields

**Requirements:**
- Efficient handling of very large files (multi-GB, millions of lines)
- Statically compiled (no runtime Qt dependencies)
- Runs on Ubuntu and RHEL

---

## 📁 Folder Structure

```
linuxtimelineviewer/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── AppWindow.h/.cpp          # Main window and menu logic
│   ├── TimelineTab.h/.cpp        # One tab = one loaded timeline
│   ├── TimelineModel.h/.cpp      # QAbstractTableModel for CSV-backed data
│   ├── TimelineParser.h/.cpp     # Format detection and CSV reader logic
│   ├── FilterBar.h/.cpp          # Reusable widget with column picker + input + search
│   └── utils/
│       ├── JsonXmlFormatter.h/.cpp  # Parses and pretty-prints embedded XML/JSON
│       └── FileUtils.h/.cpp         # Helpers: file path, base name, CSV sniffing
├── resources/
│   └── icons.qrc                # Optional: icon, stylesheet, etc.
├── data/
│   └── test_files/              # Sample timeline files for testing
└── build/                       # Recommended build directory (excluded from repo)
```

---

## 🛠 Build System Setup

**Requirements:**
- C++17
- CMake 3.16+
- Qt 5.15+ or Qt 6 (built statically)
- No non-standard libraries

**Build Instructions:**
```bash
# Clone or create project folder
mkdir -p ~/Projects/linuxtimelineviewer && cd ~/Projects/linuxtimelineviewer

# Create a build folder
mkdir build && cd build

# Set static Qt if needed
export CMAKE_PREFIX_PATH=/opt/Qt-static/lib/cmake

# Run cmake with static flags
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON

# Build
make -j$(nproc)

# Output binary will be in: bin/timeline-explorer
```

---

## 📐 Application Structure

### 1. `main.cpp`
- Initializes `QApplication`
- Loads and displays `AppWindow`

### 2. `AppWindow` (`AppWindow.h/.cpp`)
- Inherits `QMainWindow`
- Contains:
  - `QTabWidget* tabs`
  - `QMenuBar` with File → Open, Exit
  - Slot for `openFile()` uses `QFileDialog`, detects format, creates a new `TimelineTab`

### 3. `TimelineTab` (per file tab)
- **Layout:**
  ```
  +-----------------------------+
  | FilterBar (above table)     |
  | QTableView                  |
  | Status bar (rows/info)      |
  +-----------------------------+
  ```
- **Internals:**
  - Owns a `TimelineModel` and `QSortFilterProxyModel`
  - Table supports sorting on all columns
  - Search highlights matching cells

### 4. `TimelineModel` (`QAbstractTableModel`)
- Backed by file stream (not fully in memory)
- **Modes:**
  - **Filesystem Timeline:**  
    `Date,Size,Type,Mode,UID,GID,Meta,File Name`
  - **Super Timeline:**  
    `datetime,timestamp_desc,source,source_long,message,parser,display_name,tag`
- **Implements:**
  - `rowCount()`, `columnCount()`
  - `data()`, `headerData()`
  - Lazy read or line caching for performance
  - Auto-detects format in constructor via header sniffing

### 5. `FilterBar` (filter/search UI)
- Horizontal layout:
  - `QComboBox` (column names)
  - `QLineEdit` (input)
  - `QPushButton` (Search)
- Emits signal with (column, term) to be connected in `TimelineTab`
- Optional: highlight results using `QStyledItemDelegate`

### 6. `JsonXmlFormatter`
- Detects if message field contains:
  - JSON (`{...}` or `[...]`)
  - XML (`<...>`)
- Provides `QString formatIfApplicable(QString rawText)` to:
  - Return nicely indented preview string if format is detected
  - Else return original

---

## 🔍 Behavior Details

### File Type Detection
- In `TimelineParser` or `TimelineModel` constructor:
  - Read header line
  - Match columns against:
    - 8-column Filesystem Timeline
    - 8-column Super Timeline
  - Error if not recognized

### Filtering
- Uses `QSortFilterProxyModel`
- Connected to search button signal
- `setFilterKeyColumn()` and `setFilterFixedString()` used
- Highlight all matches in current view with a custom delegate (optional)

### Performance
- Try to avoid fully loading files into RAM
- Implement:
  - `std::ifstream` with offset table (line index)
  - OR memory-mapped file scan with `mmap`
- Avoid full proxy rebuilds unless absolutely necessary
- Restrict search to filtered column in version 1

---

## 🧪 Test Artifacts

Place example files in `data/test_files/`:
- `FILESYSTEM.txt` → from your example with macb and orphaned files
- `SUPER.txt` → full plaso timeline including embedded `<Event>` XML

**Test:**
```bash
./timeline-explorer data/test_files/FILESYSTEM.txt
```

---

## 📝 Code Conventions

- Use C++17, avoid boost
- Use Qt’s containers (`QString`, `QVector`, `QMap`)
- Stick to 80-100 column lines
- Document all public methods using Doxygen-compatible comments

---

## 🚧 Known Limits (v1)

- Only `.csv`/`.txt` supported
- Search is one-column at a time
- No export/save function
- No timeline alignment or graphical chart

---

## ✅ Summary: Key Developer Tasks

| Task                | File(s)                       | Notes                                 |
|---------------------|------------------------------|---------------------------------------|
| Set up project + CMake | `CMakeLists.txt`           | Static Qt, release build              |
| Create main window  | `main.cpp`, `AppWindow`      | Menubar, tabbed interface             |
| Implement tab view  | `TimelineTab`                | Holds table + filter bar              |
| Implement model     | `TimelineModel`              | Efficient file-backed CSV reader      |
| Detect format       | `TimelineModel`, `TimelineParser` | Validate columns                 |
| Create filter/search| `FilterBar`                  | Connect to model via proxy            |
| Handle JSON/XML     | `JsonXmlFormatter`           | Pretty-print inside table cell        |

