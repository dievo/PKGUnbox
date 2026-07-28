PKGUnbox — Linux Usage
======================

1. pkgunbox

   Command-line PKG extractor.

   Usage:
     pkgunbox <file.pkg> [output_dir]
     pkgunbox <file.pkg> --check-type
     pkgunbox --help

   Arguments:
     file.pkg       Path to the PS4 .pkg file to extract
     output_dir     Output directory (default: next to the .pkg file)
     --check-type   Print package type and exit (base=101, patch=102, dlc=103)


2. pkgunbox-gui

   Graphical interface (requires Qt6).


3. install_pkg.sh

   Needs to be edited before use.
   Set the values for gamesDir and addonsDir to your PS4 game/addon paths.

   Can then be called in command line:
     install_pkg.sh /path/to/game.pkg


4. batch_install.sh

   Extract all .pkg files in a directory:
     batch_install.sh /path/to/pkg/directory
