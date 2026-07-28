#!/bin/bash

gamesDir="PATH_TO_GAMES_DIR"
addonsDir="PATH_TO_ADDONS_DIR"
scriptDir="$(dirname "$0")"

if [ "$gamesDir" = "PATH_TO_GAMES_DIR" ]; then
    echo "Error: You need to set gamesDir in this script."
    echo "Example: gamesDir=\"/path/to/your/ps4/games\""
    if [ "$2" != "--batch" ]; then read -r; fi
    exit 1
fi

if [ "$addonsDir" = "PATH_TO_ADDONS_DIR" ]; then
    echo "Error: You need to set addonsDir in this script."
    echo "Example: addonsDir=\"/path/to/your/ps4/addons\""
    if [ "$2" != "--batch" ]; then read -r; fi
    exit 1
fi

if [ ! -f "$1" ]; then
    echo "Error: File not found: $1"
    if [ "$2" != "--batch" ]; then read -r; fi
    exit 1
fi

"$scriptDir/pkgunbox" "$1" --check-type
ret="$?"

case "$ret" in
    101) echo "The file is a base game, installing to $gamesDir"
         "$scriptDir/pkgunbox" "$1" "$gamesDir" ;;
    102) echo "The file is a game update, installing to $gamesDir"
         "$scriptDir/pkgunbox" "$1" "$gamesDir" ;;
    103) echo "The file is a DLC, installing to $addonsDir"
         "$scriptDir/pkgunbox" "$1" "$addonsDir" ;;
    *)   echo "Error: Failed to detect package type."
         if [ "$2" != "--batch" ]; then read -r; fi
         exit 1 ;;
esac

if [ "$2" != "--batch" ]; then read -r; fi
