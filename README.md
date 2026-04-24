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

<img width="552" height="552" alt="Bildschirmfoto_20260424_171804" src="https://github.com/user-attachments/assets/fa716230-9593-437b-9bad-6ec2dd8f9d00" />

## Download

TODO: Add Flatpak and AppImage

## Building

Required packages (Arch Linux): `base-devel extra-cmake-modules cmake kirigami ki18n kcoreaddons breeze kiconthemes qt6-base qt6-declarative qqc2-desktop-style`
In case I forgot something, cmake will tell you. Please open an issue so I can add the packages here.

### For users
1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Prepare Build directory: `cmake -B build -G Ninja`
4. Build the project: `cmake --build build --config Release`
5. Install the project: `sudo cmake --install build`

### For developers
1. Clone repository: `git clone https://github.com/Berny23/virtual-surround-manager.git`
2. Change directory: `cd virtual-surround-manager`
3. Prepare Build directory and set local install path: `cmake -B build -G Ninja --install-prefix ~/.local`
4. Build the project: `cmake --build build --config Debug`
5. Install the project: `cmake --install build`

## Acknowledgements

- The [EasyEffects](https://github.com/wwmm/easyeffects) project for research on the API, because the PipeWire docs are quite lacking for beginners
- The [HeSuVi](https://hesuvi.net) project for their awesome collection of HRIR WAV files
