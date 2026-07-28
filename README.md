# PKGUnbox

*"Unbox your PS4 games"*

A standalone tool to extract PS4 `.pkg` files — base games, updates, and DLCs.

## Features

- Extract PS4 `.pkg` files (base games, updates, DLCs)
- Automatic PKG type detection
- GUI with drag-and-drop support (Qt6)
- Dark theme (Catppuccin Mocha)
- Progress bar during extraction

## CLI Usage

```bash
# Extract a PKG file
./pkgunbox /path/to/file.pkg [output_dir]

# Check PKG type only (returns 101=base, 102=update, 103=DLC)
./pkgunbox /path/to/file.pkg --check-type
```

If no output directory is provided, the PKG is extracted next to the source file.

On Windows, you can drag and drop the `.pkg` file onto the `.exe`.

## GUI Usage

```bash
./pkgunbox-gui
```

Or use the wrapper script (fixes Snap/libpthread conflicts):

```bash
./run_gui.sh
```

## Install Scripts

The `install_pkg.sh` / `install_pkg.bat` scripts auto-install extracted PKGs into the correct directories.

**Edit the scripts first** — set `gamesDir` and `addonsDir` to your PKGUnbox paths.

## Building

```bash
cd extractor/build
git submodule update --init --recursive
./buildall        # first time: lconf → cryptobuild → lbuild

# Rebuild after code changes
./lbuild
```

Requires: CMake >= 3.5, C++23 compiler, Qt6 (for GUI), zlib.

## License

GPL-2.0-or-later
