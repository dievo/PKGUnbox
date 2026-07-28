#!/bin/bash
set -e

# PKGUnbox AppImage Builder
# Runs from: shadPS4Plus/extractor/build/

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$(dirname "$(dirname "$PROJECT_DIR")")"
BUILD_DIR="$SCRIPT_DIR"
APPDIR="$BUILD_DIR/AppDir"

LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"

echo "=== PKGUnbox AppImage Builder ==="
echo ""

# Step 1: Build if needed
if [ ! -f "$BUILD_DIR/pkgunbox-gui" ]; then
    echo ">>> Building pkgunbox and pkgunbox-gui..."
    cd "$BUILD_DIR"
    ./lconf
    ./cryptobuild
    ./lbuild
fi

# Step 2: Download linuxdeploy tools
echo ">>> Checking linuxdeploy tools..."
cd "$BUILD_DIR"

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

# Step 3: Create AppDir structure
echo ">>> Creating AppDir structure..."
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/icons/hicolor/512x512/apps"
mkdir -p "$APPDIR/usr/share/applications"

# Step 4: Copy binaries
echo ">>> Copying binaries..."
cp "$BUILD_DIR/pkgunbox" "$APPDIR/usr/bin/"
cp "$BUILD_DIR/pkgunbox-gui" "$APPDIR/usr/bin/"

# Step 5: Copy and resize icon
echo ">>> Copying icon..."
python3 -c "
from PIL import Image
img = Image.open('$SRC_DIR/docs/logo/logo.png')
img_resized = img.resize((512, 512), Image.LANCZOS)
img_resized.save('$APPDIR/usr/share/icons/hicolor/512x512/apps/pkgunbox.png')
print('    Resized logo to 512x512')
"

# Step 6: Copy desktop file
echo ">>> Copying desktop file..."
cp "$PROJECT_DIR/pkgunbox.desktop" "$APPDIR/usr/share/applications/"

# Step 7: Create symlink for AppRun
ln -sf usr/bin/pkgunbox-gui "$APPDIR/AppRun"

# Step 8: Run linuxdeploy with Qt plugin
echo ">>> Running linuxdeploy (this may take a while)..."
cd "$BUILD_DIR"

export DEPLOY_GTK_VERSION=3
./linuxdeploy-x86_64.AppImage \
    --appdir "$APPDIR" \
    --desktop-file "$APPDIR/usr/share/applications/pkgunbox.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/512x512/apps/pkgunbox.png" \
    --plugin qt \
    --output appimage

# Step 9: Rename output
APPIMAGE_FILE=$(ls -1 PKGUnbox*.AppImage 2>/dev/null | head -1)
if [ -n "$APPIMAGE_FILE" ]; then
    echo ""
    echo "=== Success! ==="
    echo "AppImage: $BUILD_DIR/$APPIMAGE_FILE"
    echo "Size: $(du -h "$APPIMAGE_FILE" | cut -f1)"
else
    echo "ERROR: AppImage not found after build"
    exit 1
fi
