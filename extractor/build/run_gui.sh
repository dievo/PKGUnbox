#!/bin/bash
# Wrapper to avoid Snap's libpthread conflict
# The snap runtime injects /snap/core20/... paths that conflict with system glibc
unset LD_LIBRARY_PATH
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$SCRIPT_DIR/pkgunbox-gui" "$@"
