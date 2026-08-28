# Building Genexis

Requirements:

A working Qt6 Kirigami environment
- kde-builder
- craft

An up to date compiler that supports C++23
- GCC 14.2+
- Clang 19.1+ 

## Windows

Using Craft. Genexis craft blueprint available in [todo]

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