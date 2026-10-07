# Building Genexis

Requirements:

A working Qt6 Kirigami environment
- kde-builder
- craft

At least Qt 6.8, KDE Frameworks 6 (Kirigami, I18n, CoreAddons, IconThemes,
QQC2DesktopStyle), extra-cmake-modules and CMake 3.29.

An up to date compiler that supports C++23
- GCC 14.2+
- Clang 19.1+
- MSVC 19.35+ (Visual Studio 2022 17.5+)

Also required:
- Git submodules: `git submodule update --init --recursive`
- Rust and cargo, for NandProMax (<https://rustup.rs>). If cargo is not on
  `PATH`, pass `-DCARGO_EXECUTABLE=/path/to/cargo`.
- libudev and libusb-1.0 (Linux), libftdi1, zlib and pkg-config
- Network access on the first configure: UpdClient fetches CLI11,
  nlohmann_json and tl::expected from GitHub with CMake FetchContent.
- ccache is optional and used automatically when found.

## Configure, build and test

```sh
git submodule update --init --recursive
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 -m unittest discover -s tests -v
```

`BUILD_TESTING` defaults to `ON`; pass `-DBUILD_TESTING=OFF` to skip the
test executables. The bundled libraries' own test suites are never built.
NandProMax is built by cargo into `<build dir>/nandpromax`.

On Linux and macOS, `./build.sh` runs the configure and build steps. It reads
`BUILD_DIR` (default `build`), `BUILD_TYPE` (default `Debug`) and `JOBS`
(default: all CPUs), and passes extra arguments to CMake:

```sh
BUILD_TYPE=Release ./build.sh -DBUILD_TESTING=OFF
```

`./clean_build.sh` deletes the build directory first. It only deletes a
directory that contains a `CMakeCache.txt`.

If an older build directory fails to link `genexis` with "hidden symbol
`ftdi_usb_purge_rx_buffer' ... is referenced by DSO", delete
`<build dir>/bin/libftdi1.so*`. Earlier configurations built that unusable
shared copy of the bundled libftdi; the current one only builds it static.

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

Rust is not listed below; install it with [rustup](https://rustup.rs) or your
distro's `cargo` package.

Ubuntu 26.04 or newer (older releases lack Qt 6.8 and KDE Frameworks 6)
```bash
sudo apt install build-essential cmake ninja-build git pkg-config extra-cmake-modules qt6-base-dev qt6-base-dev-tools qt6-declarative-dev qt6-declarative-dev-tools qt6-serialport-dev libkirigami-dev qml6-module-org-kde-kirigami libkf6i18n-dev libkf6coreaddons-dev libkf6iconthemes-dev libkf6qqc2desktopstyle-dev libftdi1-dev libudev-dev libusb-1.0-0-dev zlib1g-dev
```

Arch-based
```bash
sudo pacman -S base-devel extra-cmake-modules cmake ninja git kirigami ki18n kcoreaddons breeze kiconthemes qt6-base qt6-declarative qt6-serialport qqc2-desktop-style libftdi libusb systemd-libs zlib pkgconf
```

OpenSUSE
```bash
sudo zypper install cmake ninja git kf6-extra-cmake-modules kf6-kirigami-devel kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kiconthemes-devel qt6-base-devel qt6-declarative-devel qt6-quickcontrols2-devel qt6-serialport-devel kf6-qqc2-desktop-style libftdi1-devel libusb-1_0-devel libudev-devel zlib-devel pkg-config
```

Fedora
```bash
sudo dnf install @development-tools @development-libs cmake ninja-build git extra-cmake-modules kf6-kirigami-devel kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kiconthemes-devel qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtserialport-devel kf6-qqc2-desktop-style libftdi-devel libusb1-devel systemd-devel zlib-devel pkgconf-pkg-config
```
