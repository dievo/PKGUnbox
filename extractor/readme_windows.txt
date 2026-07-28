PKGUnbox — Windows Usage
========================

1. pkgunbox.exe

   Command-line PKG extractor.

   Usage:
     pkgunbox.exe <file.pkg> [output_dir]
     pkgunbox.exe <file.pkg> --check-type
     pkgunbox.exe --help

   Arguments:
     file.pkg       Path to the PS4 .pkg file to extract
     output_dir     Output directory (default: next to the .pkg file)
     --check-type   Print package type and exit (base=101, patch=102, dlc=103)

   Alternatively, drag and drop the .pkg file onto the .exe to extract it.


2. pkgunbox-gui.exe

   Graphical interface (requires Qt6).


3. install_pkg.bat

   Needs to be edited before use.
   Set the values for gamesDir and addonsDir to your PS4 game/addon paths.

   Can then be called in command line:
     install_pkg.bat "D:\path\to\game.pkg"

   Alternatively, drag and drop a .pkg file onto the .bat to install it.


4. batch_install.bat

   Extract all .pkg files in a directory:
     batch_install.bat "D:\path\to\pkg\directory"
