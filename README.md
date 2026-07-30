# PKGUnbox

*"Unbox your PS4 games"*

A standalone tool to extract PS4 `.pkg` files — base games, updates, and DLCs.

## Why PKGUnbox?

The [shadPS4 emulator](https://github.com/shadps4-emu/shadPS4) is the leading
PlayStation 4 emulator for Windows, Linux, and macOS. As shadPS4 matured, its
development team made a deliberate decision: **remove PKG extraction from the
emulator core** and focus exclusively on running games.

This left a gap: users had no built-in way to extract `.pkg` files (games,
updates, DLCs) into the format shadPS4 expects. PKGUnbox fills that gap.

```
┌─────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  PKGUnbox        │ ──▶ │  shadPS4         │ ──▶ │  Play!           │
│  (extract .pkg)  │     │  (run the game)  │     │  (enjoy)         │
└─────────────────┘     └──────────────────┘     └──────────────────┘
```

**Key facts:**

- PKGUnbox is **fully independent** of shadPS4 — it has its own PKG parser
  and crypto implementation.
- The official shadPS4 repository no longer includes PKG extraction code.
- The PKG format is defined by Sony and is stable. PKGUnbox will continue
  to work regardless of how shadPS4 evolves.

## Download

Go to [Releases](../../releases) and download the latest version for your platform.

| Platform | File | Requirements |
|----------|------|-------------|
| **Linux** | `PKGUnbox-x86_64.AppImage` | None (self-contained) |
| **Windows** | `PKGUnbox-windows-x64.zip` | None (self-contained) |

## Quick Start

### Linux (AppImage)

1. Download `PKGUnbox-x86_64.AppImage` from [Releases](../../releases)
2. Make it executable:
   ```bash
   chmod +x PKGUnbox-x86_64.AppImage
   ```
3. Run it:
   ```bash
   # Open the GUI
   ./PKGUnbox-x86_64.AppImage

   # Or extract a PKG directly from the terminal
   ./PKGUnbox-x86_64.AppImage /path/to/file.pkg /path/to/output/
   ```

### Windows

1. Download `PKGUnbox-windows-x64.zip` from [Releases](../../releases)
2. Extract the `.zip` anywhere
3. Run it:
   - Double-click `pkgunbox-gui.exe` for the GUI
   - Or open a terminal and run:
     ```
     pkgunbox.exe C:\path\to\file.pkg C:\path\to\output\
     ```

## Features

- Extract PS4 `.pkg` files (base games, updates, DLCs)
- Automatic PKG type detection (base = 101, update = 102, DLC = 103)
- GUI with drag-and-drop support (Qt6)
- Dark theme with blurple accent colors
- Progress bar during extraction
- Save/load output directories
- Multi-language support (English, Portugues, Espanol)
- CLI mode for power users and scripting

## CLI Reference

```bash
# Extract a PKG file to a specific directory
./pkgunbox /path/to/file.pkg /path/to/output/

# Extract to the same directory as the source file (default)
./pkgunbox /path/to/file.pkg

# Check PKG type only (returns 101=base, 102=update, 103=DLC)
./pkgunbox /path/to/file.pkg --check-type
```

### Exit codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Error (file not found, invalid PKG, etc.) |

## GUI Usage

The GUI supports:

- **Drag & drop** — drag a `.pkg` file onto the window
- **File browser** — click the drop zone to browse
- **Directory selection** — choose where to extract games and DLCs
- **Language switcher** — bottom-left corner (English, Portugues, Espanol)
- **Live log** — see extraction progress in real time

## Building from Source

### Prerequisites

- CMake >= 3.5
- C++23 compiler (GCC or Clang)
- Qt6 (Widgets, Core, Gui) — for GUI
- zlib >= 1.3
- Git (for submodule)

### Build

```bash
# Clone the repo
git clone --recurse-submodules <repo-url>
cd PKGUnbox/extractor/build

# First time only: build everything
./buildall        # runs: lconf → cryptobuild → lbuild

# Rebuild after code changes
./lbuild

# Reconfigure (after CMakeLists.txt changes)
./lconf && ./lbuild
```

### Run

```bash
# CLI
./pkgunbox /path/to/file.pkg

# GUI
./pkgunbox-gui
# or: ./run_gui.sh    (fixes Snap/libpthread conflict)
```

### Build AppImage

```bash
./build_appimage.sh                # package existing binaries
./build_appimage.sh --rebuild      # rebuild + package
./build_appimage.sh --clean        # clean + rebuild + package
```

## Platform Support

PKGUnbox uses Qt6, which defines the minimum supported platforms:

| Platform | Format | Minimum version | Notes |
|----------|--------|-----------------|-------|
| **Linux** | AppImage | glibc >= 2.35 (Ubuntu 22.04+, Fedora 36+, etc.) | Works on all modern distros. Snap users: use `run_gui.sh` to avoid libpthread conflicts. |
| **Windows** | .exe (ZIP) | Windows 10 (all versions) | Windows 7 and 8.x are **not** supported by Qt6. |

> **AppImage compatibility:** The AppImage is built on Ubuntu 24.04 and bundles
> all required Qt6 libraries. It runs on any Linux distribution with glibc >= 2.35
> — no need to install Qt6 or any other dependency.

## Related Projects & Credits

- **[shadPS4](https://github.com/shadps4-emu/shadPS4)** — the PlayStation 4
  emulator that PKGUnbox complements. PKGUnbox's extraction core was originally
  based on early shadPS4 code, before PKG parsing was removed from the emulator.
- **[shadPS4Plus](https://github.com/AzaharPlus/shadPS4Plus)** — the direct fork of
  shadPS4 that served as the base for PKGUnbox. It provided a working CLI
  PKG extractor and install scripts. PKGUnbox extends that work with a
  user-friendly Qt6 GUI.
- **[shadps4-qtlauncher](https://github.com/shadps4-emu/shadps4-qtlauncher)** —
  official Qt launcher for shadPS4 (for users who prefer a GUI over CLI).

## License

GPL-2.0-or-later
