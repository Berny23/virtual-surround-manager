<img src="src/assets/icons/de.berny23.virtual_surround_manager.svg" alt="Virtual Surround Manager icon" width="100" height="100"/>

# Virtual Surround Manager

<a href="https://www.buymeacoffee.com/Berny23" title="Donate to this project using Buy Me A Coffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-yellow.svg" alt="Buy Me A Coffee donate button" /></a>

Enable virtual 7.1 surround sound for your headphones on Linux with just one click.

## Features

- Enable or disable virtual surround sound seemlessly
- Drop in your own **HRIR WAV files** as virtualization presets, same as **HeSuVi**
- Profile selection is automatically saved
- Compatible with **EasyEffects** (and probably similar apps)
- Written entirely using the **PipeWire C++ API**, no config files are required or used
- Modern user interface, built with **KDE's Kirigami 6 framework**
- Available languages: English, German
- Project is **not** just AI-generated, everything is written by hand or autocomplete ;)

## Demo

<img width="552" height="580" alt="Bildschirmfoto_20260424_172129" src="https://github.com/user-attachments/assets/f3584099-4a28-45d5-b13a-839fa49ea62f" />

## Download

TODO: Add Flatpak and AppImage

## Building

### Native

#### Dependencies
Arch Linux: `sudo pacman -S git ninja libpipewire base-devel extra-cmake-modules cmake kirigami ki18n kcoreaddons breeze kiconthemes qt6-base qt6-declarative qqc2-desktop-style`

In case I forgot something, cmake will tell you. Please open an issue so I can add the packages here.

#### For users
1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Prepare Build directory: `cmake -B build -G Ninja`
4. Build the project: `cmake --build build --config Release`
5. Install the project: `sudo cmake --install build --config Release`

#### For developers
1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Prepare Build directory and set local install path: `cmake -B build -G Ninja --install-prefix ~/.local`
4. Build the project: `cmake --build build --config Debug`
5. Install the project: `cmake --install build --config Debug`

### Flatpak

The UI, filter chain node creation and audio source node detection work fine. However, setting the metadata for actual routing does absolutely nothing and the connection is not visible in coppwr. But this should actually work, because EasyEffects and other apps also support Flatpak.

**IF YOU ARE A DEVELOPER, PLEASE HELP ME WITH THE FLATPAK. THANKS!**

1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Build the flatpak: `flatpak-builder --force-clean --user --install-deps-from=flathub --repo=repo --install flatpak_build ./dist/flatpak/de.berny23.virtual_surround_manager.Devel.json`
4. Run the program: `flatpak run de.berny23.virtual_surround_manager`

### AppImage

I don't even know how this is supposed to work. There are mutliple different builder tools with no clear documentation on which to use. I tried some, but my build just crashes when launching it.

## Acknowledgements

- The [EasyEffects](https://github.com/wwmm/easyeffects) project for research on the API, because the PipeWire docs are quite lacking for beginners
- The [HeSuVi](https://hesuvi.net) project for their awesome collection of HRIR WAV files
