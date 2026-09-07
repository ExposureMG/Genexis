# Building Genexis

Requirements:

A working Qt6 Kirigami environment
- kde-builder
- craft

An up to date compiler that supports C++23
- GCC 14.2+
- Clang 19.1+ 
- MSVC 19.35+ (Visual Studio 2022 17.5+)

## Compiler selection

On every supported platform, a fresh CMake configuration prefers `clang` and
`clang++`, then falls back to `gcc` and `g++` if a complete Clang pair is not
available. On Windows, if neither is found or when running in a Visual Studio
environment, CMake defaults to MSVC (`cl.exe`). Put the desired compiler binaries on `PATH`.
Fallback is based on compiler availability, not a failed compilation or an old
compiler version; the selected compiler and standard library must support C++23.

Explicit `CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER` settings, `CC` / `CXX`, and
toolchain files are left to CMake and checked after compiler identification.
When overriding the defaults, specify both C and C++ compilers. Version-suffixed
binaries (for example, `clang-19` and `clang++-19`) can be selected this way too.
Existing build directories retain their cached compilers; use a new build
directory when switching compilers.

## Windows

Using Craft. Genexis craft blueprint is available in [genexis.py](genexis.py).

For manual builds with Ninja or MSVC:

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

Or when using the Visual Studio generator:

```sh
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Release
```

No compiler flags are needed when the desired compiler is on `PATH` and
no explicit compiler or toolchain override is set. Run in an environment where
CMake can find the Qt/KDE dependencies (for example, a suitably configured Craft
shell).

1. [Craft Setup](https://develop.kde.org/docs/getting-started/building/craft/)
2. [Kirigami Setup](https://develop.kde.org/docs/getting-started/kirigami/platforms-windows/)

## MacOS and Android

Craft is also supported but there is no setup tutorial for either platform.

[Craft Setup](https://develop.kde.org/docs/getting-started/building/craft/)

## Linux

CMake is pre-configured but dependencies are required. KDE Builder will do everything automatically, or you can use your distro's packages.

1. [KDE Builder Setup](https://develop.kde.org/docs/getting-started/building/kde-builder-setup/)


2. Distro Packages

Ubuntu-based
```bash	
sudo apt install build-essential cmake extra-cmake-modules libkirigami-dev libkf6i18n-dev libkf6coreaddons-dev libkf6iconthemes-dev qt6-base-dev qt6-declarative-dev libkf6qqc2desktopstyle-dev
```

Arch-based
```bash
sudo pacman -S base-devel extra-cmake-modules cmake kirigami ki18n kcoreaddons breeze kiconthemes qt6-base qt6-declarative qqc2-desktop-style
```

OpenSUSE
```bash
sudo zypper install cmake kf6-extra-cmake-modules kf6-kirigami-devel kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kiconthemes-devel qt6-base-devel qt6-declarative-devel qt6-quickcontrols2-devel kf6-qqc2-desktop-style
```

Fedora
```bash
sudo dnf install @development-tools @development-libs cmake extra-cmake-modules kf6-kirigami-devel kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kiconthemes-devel qt6-qtbase-devel qt6-qtdeclarative-devel kf6-qqc2-desktop-style
```
