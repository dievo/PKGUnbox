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
- PKGUnbox was originally forked from an early version of shadPS4 that still
  had PKG code. The emulator code was removed, leaving only the extraction
  core, which was then cleaned up and wrapped in a user-friendly GUI.

## Features

- Extract PS4 `.pkg` files (base games, updates, DLCs)
- Automatic PKG type detection (base = 101, update = 102, DLC = 103)
- GUI with drag-and-drop support (Qt6)
- Dark theme (Catppuccin Mocha)
- Progress bar during extraction
- Save/load output directories (QSettings)
- Multi-language support (English, Portugues, Espanol)
- CLI mode for power users and scripting

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

**Edit the scripts first** — set `gamesDir` and `addonsDir` to your shadPS4 paths.

## Building

```bash
cd extractor/build
git submodule update --init --recursive
./buildall        # first time: lconf → cryptobuild → lbuild

# Rebuild after code changes
./lbuild
```

Requires: CMake >= 3.5, C++23 compiler, Qt6 (for GUI), zlib.

## Related Projects & Credits

- **[shadPS4](https://github.com/shadps4-emu/shadPS4)** — the PlayStation 4
  emulator that PKGUnbox complements. PKGUnbox's extraction core was originally
  based on early shadPS4 code, before PKG parsing was removed from the emulator.
- **[shadPS4Plus](https://github.com/AzaharPlus/shadPS4Plus)** — the direct fork of
  shadPS4 that served as the base for PKGUnbox. It provided a working CLI
  PKG extractor and install scripts. PKGUnbox extends that work with a
  user-friendly Qt6 GUI. All credits to the shadPS4Plus team for the initial
  CLI extraction work.
- **[shadps4-qtlauncher](https://github.com/shadps4-emu/shadps4-qtlauncher)** —
  official Qt launcher for shadPS4 (for users who prefer a GUI over CLI).

## License

GPL-2.0-or-later
