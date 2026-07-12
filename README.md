<h2 align="center">
  <img src="src/assets/icons/de.berny23.virtual_surround_manager.svg" alt="Virtual Surround icon" width="160" height="160"/>
  <br>
  Virtual Surround
</h2>

<p align="center">
  <strong>3D sound for headphones on Linux</strong>
</p>

<p align="center">
<a href="https://www.buymeacoffee.com/Berny23" title="Donate to this project using Buy Me A Coffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-yellow.svg" alt="Buy Me A Coffee donate button" /></a>
</p>

## Features

- Enable or disable seemlessly, without changing your default device
- Drop in your own **HRIR WAV files** as virtualization presets, same as **HeSuVi** on Windows
- Can be run in the background with a **system tray icon** (check the app's settings)
- Compatible with **EasyEffects** (and most similar apps)
- Written entirely using the **PipeWire C API**, so no config files or annoying service restarts are required
- Modern user interface, built with **KDE's Kirigami 6 framework**
- Available languages are English, German, Russian, Italian and French
- This project is **not using** vibe coding, AI agents or similar tools for generating unmaintainable slop

## Screenshot

<img width="552" height="580" alt="Screenshot of this app" src="dist/flatpak/screenshots/main-window-dark.webp" />

## Installation

### Flatpak (Recommended)

Compatible with any distribution.

<a href='https://flathub.org/en/apps/de.berny23.virtual_surround_manager'><img width='240' alt='Get Virtual Surround on Flathub' src='https://flathub.org/api/badge?svg&locale=en'/></a>

### AppImage

Compatible with any distribution.
Just [Download](https://github.com/Berny23/virtual-surround-manager/releases/latest) and run. Use the "x86_64" file if you're unsure.

### AUR

For Arch Linux and derivatives. Install with your AUR manager, like: `yay -S virtual-surround-manager`

### Third-party packages

These packages are maintained and provided by community members.

- [Fedora](https://software.opensuse.org/download/package?package=virtual-surround-manager&project=home%3AAndnoVember%3AFedora) (by AndnoVember)
- [Ubuntu/Debian](https://software.opensuse.org/download.html?package=virtual-surround-manager&project=home%3AAndnoVember%3ADebian) (by AndnoVember)
- [OpenSUSE](https://software.opensuse.org/download/package?package=virtual-surround-manager&project=home%3AAndnoVember%3ALXQt%3AQt6) (by AndnoVember)
- [Arch Linux](https://software.opensuse.org/download/package?package=virtual-surround-manager&project=home%3AAndnoVember%3AArch) (by AndnoVember)

## Usage

Just open the settings of your favorite game or media player and select real 7.1 or 5.1 audio output. Do not use any additional in-game virtualization setting (like a “headphone” or "3D audio" profile).

Please note: This app requires the PipeWire audio system. Recent Linux distributions use this by default, so don't worry about it.

### Usage with EasyEffects and similar apps

**TL;DR:**
<br>
In EasyEffects:
- Tick "Enable" for Virtual Surround Manager
- Tick "Exclude" for the actual game/app

Detailed explanation:

Both EasyEffects and Virtual Surround do essentially the same thing, they re-route audio. Meaning, **you must exclude the actual 7.1/5.1 game from routing in EasyEffects**, so my app can process it first!

**How routing works with EasyEffects for example:**
Game "Overwatch" (7.1/5.1 channels) → Virtual Surround Sink (7.1/5.1 channels) → Virtual Surround Source (2 channels) → Easy Effects Sink (2 channels) → All your EasyEffects plugins → Headphones (2 channels)

Note: The spatial virtualization and conversion to stereo happens between "Virtual Surround Sink" and "Virtual Surround Source".

**Apply all your effects on Virtual Surround in EasyEffects, not the game itself!**
If you want to exclude one app from Virtual Surround (like a music player or Discord), just tick "Enabled" for the specific app to route it directly through EasyEffects and not Virtual Surround.

### Check internal connections

To check if everything is working correctly, this is how audio routing should look like in [coppwr](https://flathub.org/de/apps/io.github.dimtpap.coppwr). If you don't use EasyEffects, the "Virtual Surround Source" should be directly connected to your headphones.
<br>
<img width="3401" height="849" alt="Bildschirmfoto_20260712_182628" src="https://github.com/user-attachments/assets/1bdb0cec-f318-4f50-9469-1ef502a2bc19" />

## Building

### Native

#### Dependencies for building the native package or AppImage

Arch Linux: `sudo pacman -S git ninja libpipewire base-devel extra-cmake-modules cmake kirigami ki18n kcoreaddons breeze kiconthemes qt6-base qt6-declarative qqc2-desktop-style`

For other distributions, just look up how the packages are called in your distro: https://pkgs.org

#### Native (for users)

1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Prepare Build directory: `cmake -B build -G Ninja`
4. Build the project: `cmake --build build --config Release`
5. Install the project: `sudo cmake --install build --config Release`

#### Native (for developers)

1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Prepare Build directory and set local install path: `cmake -B build -G Ninja --install-prefix ~/.local`
4. Build the project: `cmake --build build --config Debug`
5. Install the project: `cmake --install build --config Debug`

### Flatpak

1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Build the flatpak: `flatpak-builder --force-clean --user --install-deps-from=flathub --repo=repo --install flatpak_build ./dist/flatpak/de.berny23.virtual_surround_manager.Devel.json`
4. Run the program: `flatpak run de.berny23.virtual_surround_manager`

### AppImage

1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Build the AppImage: `./dist/appimage/build_virtual_surround_manager.sh`
4. Make executable: `chmod +x ./build_appimage/virtual-surround-manager-unknown-x86_64.AppImage`
5. Run the program: `./build_appimage/virtual-surround-manager-unknown-x86_64.AppImage`

## Acknowledgements

- The [EasyEffects](https://github.com/wwmm/easyeffects) project for research on the API, because the PipeWire docs are quite lacking for beginners
- The [HeSuVi](https://hesuvi.net) project for their awesome collection of HRIR WAV files
