cmake_minimum_required(VERSION 3.28)

project("Virtual Surround Manager" VERSION 1.0.0  LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)

add_subdirectory(src)