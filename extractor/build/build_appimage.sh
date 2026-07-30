#!/bin/bash
set -e

# PKGUnbox AppImage Builder
# Usage: ./build_appimage.sh [--rebuild] [--clean]
#
# Options:
#   --rebuild   Force rebuild binaries before packaging
#   --clean     Remove AppDir and previous AppImage before building
#
# Runs from: PKGUnbox/extractor/build/

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$SCRIPT_DIR"
APPDIR="$BUILD_DIR/AppDir"

DO_REBUILD=0
DO_CLEAN=0

for arg in "$@"; do
    case "$arg" in
        --rebuild) DO_REBUILD=1 ;;
        --clean)   DO_CLEAN=1 ;;
        --help|-h)
            echo "Usage: $0 [--rebuild] [--clean]"
            echo "  --rebuild   Force rebuild binaries before packaging"
            echo "  --clean     Remove AppDir and previous AppImage before building"
            exit 0
            ;;
    esac
done

echo "=== PKGUnbox AppImage Builder ==="
echo "Project:  $PROJECT_DIR"
echo "Build:    $BUILD_DIR"
echo ""

# --- Clean if requested ---
if [ "$DO_CLEAN" -eq 1 ]; then
    echo ">>> Cleaning previous build artifacts..."
    rm -rf "$APPDIR"
    rm -f "$BUILD_DIR"/PKGUnbox*.AppImage
fi

# --- Step 1: Build if needed ---
if [ "$DO_REBUILD" -eq 1 ] || [ ! -f "$BUILD_DIR/pkgunbox-gui" ]; then
    echo ">>> Building pkgunbox and pkgunbox-gui..."
    cd "$BUILD_DIR"
    if [ "$DO_REBUILD" -eq 1 ]; then
        ./lconf
        ./cryptobuild
    fi
    ./lbuild
    echo ""
fi

# Verify binaries exist
if [ ! -f "$BUILD_DIR/pkgunbox" ] || [ ! -f "$BUILD_DIR/pkgunbox-gui" ]; then
    echo "ERROR: Binaries not found. Run '$0 --rebuild' first."
    exit 1
fi

# --- Step 2: Download linuxdeploy tools ---
echo ">>> Checking linuxdeploy tools..."
cd "$BUILD_DIR"

LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"

if [ ! -f "$BUILD_DIR/linuxdeploy-x86_64.AppImage" ]; then
    echo "    Downloading linuxdeploy..."
    wget -q "$LINUXDEPLOY_URL" -O "$BUILD_DIR/linuxdeploy-x86_64.AppImage"
    chmod +x "$BUILD_DIR/linuxdeploy-x86_64.AppImage"
fi

if [ ! -f "$BUILD_DIR/linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
    echo "    Downloading linuxdeploy-plugin-qt..."
    wget -q "$LINUXDEPLOY_QT_URL" -O "$BUILD_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "$BUILD_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"
fi

# --- Step 3: Create AppDir structure ---
echo ">>> Creating AppDir structure..."
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/icons/hicolor/512x512/apps"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/pkgunbox/translations"

# --- Step 4: Copy binaries ---
echo ">>> Copying binaries..."
cp "$BUILD_DIR/pkgunbox" "$APPDIR/usr/bin/"
cp "$BUILD_DIR/pkgunbox-gui" "$APPDIR/usr/bin/"

# --- Step 4b: Copy translations (.qm files) ---
echo ">>> Copying translations..."
QM_DIR="$BUILD_DIR/translations"
if [ -d "$QM_DIR" ] && ls "$QM_DIR"/pkgunbox_*.qm 1>/dev/null 2>&1; then
    cp "$QM_DIR"/pkgunbox_*.qm "$APPDIR/usr/share/pkgunbox/translations/"
    echo "    Copied $(ls "$APPDIR/usr/share/pkgunbox/translations/"pkgunbox_*.qm | wc -l) translation files"
else
    echo "    WARNING: No .qm files found in $QM_DIR"
    echo "    Translations will not be available in the AppImage"
fi

# --- Step 5: Copy and resize icon ---
echo ">>> Copying icon..."
ICON_SRC="$PROJECT_DIR/icon.png"
if [ ! -f "$ICON_SRC" ]; then
    echo "    ERROR: icon.png not found at $ICON_SRC"
    exit 1
fi
python3 -c "
from PIL import Image
img = Image.open('$ICON_SRC')
img_resized = img.resize((512, 512), Image.LANCZOS)
img_resized.save('$APPDIR/usr/share/icons/hicolor/512x512/apps/pkgunbox.png')
print('    Resized icon.png (1024x1024) -> 512x512')
"

# --- Step 6: Copy desktop file ---
echo ">>> Copying desktop file..."
cp "$PROJECT_DIR/pkgunbox.desktop" "$APPDIR/usr/share/applications/"

# --- Step 7: Create symlink for AppRun ---
ln -sf usr/bin/pkgunbox-gui "$APPDIR/AppRun"

# --- Step 8: Run linuxdeploy with Qt plugin ---
echo ">>> Running linuxdeploy (this may take a while)..."
cd "$BUILD_DIR"

export DEPLOY_GTK_VERSION=3
./linuxdeploy-x86_64.AppImage \
    --appdir "$APPDIR" \
    --desktop-file "$APPDIR/usr/share/applications/pkgunbox.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/512x512/apps/pkgunbox.png" \
    --plugin qt \
    --output appimage

# --- Step 9: Rename output with version ---
APPIMAGE_FILE=$(ls -1 PKGUnbox*.AppImage 2>/dev/null | head -1)
if [ -n "$APPIMAGE_FILE" ]; then
    # Try to get version from git tag, fallback to commit hash
    VERSION=""
    if git -C "$PROJECT_DIR" describe --tags --exact-match HEAD 2>/dev/null; then
        VERSION=$(git -C "$PROJECT_DIR" describe --tags --exact-match HEAD)
    elif git -C "$PROJECT_DIR" rev-parse --short HEAD 2>/dev/null; then
        VERSION="dev-$(git -C "$PROJECT_DIR" rev-parse --short HEAD)"
    fi

    if [ -n "$VERSION" ]; then
        FINAL_NAME="PKGUnbox-x86_64-${VERSION}.AppImage"
        mv "$BUILD_DIR/$APPIMAGE_FILE" "$BUILD_DIR/$FINAL_NAME"
        APPIMAGE_FILE="$FINAL_NAME"
    fi

    echo ""
    echo "=== Success! ==="
    echo "AppImage: $BUILD_DIR/$APPIMAGE_FILE"
    echo "Size:     $(du -h "$BUILD_DIR/$APPIMAGE_FILE" | cut -f1)"
    echo ""
    echo "Test it:  ./$APPIMAGE_FILE"
else
    echo "ERROR: AppImage not found after build"
    exit 1
fi
