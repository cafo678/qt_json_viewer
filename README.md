# SimpleFileViewer (Qt 6 / QML JSON Viewer)

A small Qt 6 + QML app for opening a local `.json` file and viewing it in a tree.

This project is primarily for learning/performance experiments:
- Measuring JSON load timings (open / read / parse / model build)
- Understanding UI-thread blocking vs background work
- Exploring QML + C++ integration (properties, signals, models)

## Features
- Open local JSON files via `FileDialog`
- Displays JSON data in a QML `TreeView`
- Shows basic status text (and optional loading indicator)
- Logs timings for:
  - file open
  - file read
  - JSON parse
  - building the tree model

## Notes / Known limitations
- Building a full tree model for very large JSON files can be extremely slow and memory-heavy.
  For large inputs (hundreds of MB / GB), a lazy-loading approach is recommended.

## Requirements
- Qt 6 (tested with Qt 6.10.x)
- Qt Quick / Qt Quick Controls

## Build & Run
### Qt Creator
1. Open the project in Qt Creator
2. Configure a kit (MinGW or MSVC)
3. Build and run

### Command line (CMake)
If your project uses CMake:
```bash
cmake -S . -B build
cmake --build build
./build/<app-name>
```

## Project structure (typical)
- `Main.qml`: QML UI (menu, status panel, TreeView)
- `FileReader`: C++ backend that loads/parses JSON and builds a `QStandardItemModel` exposed to QML

## License
MIT (or choose your preferred license)
