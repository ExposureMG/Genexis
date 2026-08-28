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

Full Support

- Windows 10+
- Linux
- MacOS
- iPadOS
- Android
- BSD

Partial Support

- iPhone (No USB Flashers)

## Build

See [build](./docs/build.md)

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