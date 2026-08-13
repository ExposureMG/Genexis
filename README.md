# Genexis

WIP

Cross-platform Xbox 360 JTAG/RGH toolkit.

Features:

* NAND Builder
* BadUpdate USB Builder
* SPI, eMMC, and XSVF/SVF Flashing
* Launch.ini, Rgloader.ini & Options.ini editor
* XeLL, UpdServer, and DashLaunch Network Interaction

Planned:

* POST and UART Reading

# NAND Builders

I aim to support as many as possible; gxbuild3 is built-in for NAND info and Extraction.

- gxbuild3
- Build360 (Planned)
- xeBuild (Windows-only)
- RGBuild (Windows-only)

# Flashers

NAND / eMMC:

- xFlasher360 / Squirt Programmer
- Pi Pico (PicoFlasher v4+)
- JR-Programmer / NAND-X / Matrix SPI
- TX DemoN
- UpdServer

XSVF / SVF / Timing:

- xFlasher360
- Pi Pico (CMSIS-DAP v2) (Via NandProMax)
- Pi Pico (Pico-DirtyJTAG) (Via xsvftool)
- JR-Programmer / NAND-X / Matrix SPI
- TX DemoN

# Support

- Windows 10 or Later x64 and arm64
- Linux x86-64 and arm64
- MacOS x86-64 and aarch64
- Android armv8

# Credits

- Team Xecuter / Octal450 / Mena PhenomMod / mitchellwaite - J-Runner with Extras

Heavy inspiration for this project, Genexis uses J-Runner xeBuild folders and XeLL images.

## Backend Credits

- gxbuild3, by ExposureMG

- Build360, by Hax360 / Kmx360 / Mate

- xeBuild, by [cOz]

- FTDI2SPI, by Octal450 and Mena PhenomMod

- xsvftool, by Mena PhenomMod

- NandProMax, by ExposureMG

- UpdClient, by ExposureMG

- STFS, by erorn

- libfatx, by Matt Borgerson
