
set -eux

ARCH="$(uname -m)"
SHARUN="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/quick-sharun.sh"

export ICON=/virtual-surround-manager/src/assets/icons/de.berny23.virtual_surround_manager.svg
export DESKTOP=DUMMY
export OUTPATH=./dist
export MAIN_BIN=virtual-surround-manager
export OUTNAME=virtual-surround-manager-"$ARCH".AppImage

pacman -Syu --noconfirm \
	base-devel       \
	curl             \
	git              \
	wget             \
	xorg-server-xvfb \
	zsync \
	ninja \
	libpipewire \
	base-devel \
	extra-cmake-modules \
	cmake \
	kirigami \
	ki18n \
	kcoreaddons \
	breeze \
	kiconthemes \
	qt6-base \
	qt6-declarative \
	qqc2-desktop-style

echo "Building Application"
echo "---------------------------------------------------------------"

git clone https://github.com/Berny23/virtual-surround-manager.git
cd virtual-surround-manager
cmake -B build -G Ninja
cmake --build build --config Release
cmake --install build --config Release

echo "Bundling AppImage..."
echo "---------------------------------------------------------------"
wget --retry-connrefused --tries=30 "$SHARUN" -O ./quick-sharun
chmod +x ./quick-sharun
./quick-sharun /usr/bin/virtual-surround-manager

./quick-sharun --make-appimage
