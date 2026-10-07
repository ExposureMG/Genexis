# Genexis

WIP

Cross-platform Xbox 360 JTAG/RGH toolkit

## Info

The goal is to re-implement most of what J-Runner can do, cross-platform, in C++.

- NAND Info
- NAND Building
- NAND Flashing
- SVF / XSVF Flashing
- Timings Interface
- UART and POST
- XeLL Network

Flashers:

- PicoFlasher v4+
- xFlasher360 / Squirt Programmer
- TX JR-Programmer v1/v2
- TX NAND-X
- TX Demon
- xeBuild / DashLaunch UpdServ

## Platforms

| Platform | Status |
| --- | --- |
| Linux | Supported; built and tested by the CI workflow |
| Windows 10+ | Supported via KDE Craft (MSVC); not covered by CI |
| macOS | Accepted by CMake, not yet tested |
| Android | Accepted by CMake, not yet tested |
| iPadOS | Planned |
| iPhone | Planned (no USB flashers) |
| BSD | Planned (CMake currently stops with "Unsupported operating system") |

## Build

Quick start on Linux, once the dependencies in [docs/build.md](./docs/build.md)
are installed:

```bash
git submodule update --init --recursive
./build.sh
```

See [build](./docs/build.md) for prerequisites, options and other platforms.

## Developer Info

**Stack:**

- C++23 CMake
- Qt6 QML
- KDE Frameworks Kirigami

**Libraries Used:**

- [gxbuild3](https://github.com/ExposureMG/gxbuild3.git)
- [FTDI2SPI](https://github.com/ExposureMG/FTDI2SPI.git) (Experimental libftdi version)
- [xsvftool](https://github.com/ExposureMG/xsvftool.git) (Experimental libftdi version)
- [NandProMax](https://github.com/ExposureMG/NandProMax.git)
- [UpdClient](https://github.com/ExposureMG/UpdClient.git)