# Genexis

Cross-platform Xbox 360 JTAG/RGH toolkit.

## Features

* NAND Builder
* SPI, eMMC, and XSVF/SVF Flashing
* UpdServer Network Flashing
* BadUpdate USB Builder

Planned:

* Launch.ini, Rgloader.ini & Options.ini editor
* POST and UART Reading

## Support

Genexis should compile for all Linux and BSD distros, but we only provide official support for Debian/Ubuntu, Fedora, and Arch Linux.

Genexis should also compile for x86-32, x86-64, and arm64.

| OS                 | Support                                              | 32-bit | 64-bit | arm64    | Compilers        |
| ------------------ | ---------------------------------------------------- | ------ | ------ | -------- | ---------------- |
| Windows 10+        | ✅                                                   | ✅     | ✅     | ✅       | MSVC, GCC, Clang |
| Windows Vista SP2+ | [Genexis Qt5](https://github.com/GxOSS/Genexis-qt5)] | ✅     | ✅     | ⚠️       | MinGW            |
| Linux              | ✅                                                   | ✅     | ✅     | ✅       | GCC, Clang       |
| MacOS              | ✅                                                   | ⚠️     | ✅     | ✅       | Clang            |
| Android            | ✅                                                   | ⚠️     | ⚠️     | ✅       | Clang            |
| IPadOS             | ✅                                                   | ⚠️     | ⚠️     | M-Series | Clang            |

## Devices

NAND / eMMC Flashing:

- xFlasher360
- PicoFlasher v4+
- JR-Programmer
- NAND-X
- TX DemoN
- UpdServ

XSVF / SVF / Timing:

- xFlasher360
- Pico-DirtyJTAG
- JR-Programmer
- NAND-X
- TX DemoN


## Integrations

NAND Builders:

- Build360 (Planned)
- gxbuild3 (Built-in)
- xeBuild (Windows-only)

Flashers:

- NandProMax
- FTDI2SPI
- xsvftool


## License

Genexis is brought to you under the GNU General Public License Version 2.0.

See [LICENSE](./LICENSE)

Before release:

- Statically link xsvftool
- Statically link FTDI2SPI


## Credits

- Team Xecuter / Octal450 / Mena PhenomMod / mitchellwaite - J-Runner with Extras

Heavy inspiration for this project, Genexis uses J-Runner xeBuild folders and XeLL images.


## Backend Credits

- gxbuild3, by ExposureMG

- Build360, by Mate Kukri

- xeBuild, by [cOz]

- FTDI2SPI, by Swizzy, Octal450 and Mena

- xsvftool, by Mena PhenomMod

- NandProMax, by ExposureMG

- UpdClient, by ExposureMG
