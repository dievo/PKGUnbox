#!/bin/bash
# PKGUnbox AppImage wrapper
# Detects whether to launch CLI or GUI based on arguments

DIR="$(dirname "$0")"
CLI="$DIR/usr/bin/pkgunbox"
GUI="$DIR/usr/bin/pkgunbox-gui"

# If arguments provided → CLI mode
if [ $# -gt 0 ]; then
    exec "$CLI" "$@"
else
    exec "$GUI"
fi
