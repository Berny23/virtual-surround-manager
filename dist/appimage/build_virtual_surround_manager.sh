#!/bin/sh

set -eu

ARCH=$(uname -m)
SHARUN="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/quick-sharun.sh"

export ARCH
export ICON=./build_install/share/icons/hicolor/scalable/apps/de.berny23.virtual_surround_manager.svg
export DESKTOP=./build_install/share/applications/de.berny23.virtual_surround_manager.desktop
export OUTPATH=./build_appimage
export MAIN_BIN=virtual-surround-manager
export OUTNAME=virtual-surround-manager-"$ARCH".AppImage

echo "Building Application"
echo "---------------------------------------------------------------"

cmake -B build -G Ninja -DIS_APPIMAGE=1 --install-prefix $(pwd)/build_install
cmake --build build --config Release
cmake --install build --config Release

echo "Bundling AppImage..."
echo "---------------------------------------------------------------"
wget --retry-connrefused --tries=30 "$SHARUN" -O ./quick-sharun.sh
chmod +x ./quick-sharun.sh
./quick-sharun.sh ./build_install/bin/virtual-surround-manager

./quick-sharun.sh --make-appimage
