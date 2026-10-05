#pragma once

// C ABI exported by the FTDI2SPI shared library (extern/FTDI2SPI/src/Main.cpp).
// The submodule ships no public header, so the subset Genexis uses is declared
// here. Keep in sync with the FTDI2SPI_EXPORT functions in Main.cpp.

extern "C" {

// mode: 0 = read flash config only, 1 = read (0x210 pages), 2 = read (0x200
// pages), 3 = write, 4 = write with ECC patch, 5 = erase.
// size is in MB and only applies when length == 0; startblock and length are
// in 16 KB units (32 pages).
int spi(int mode, int size, char *file, int startblock, int length);
int emmc_read(const char *file, int startblock, int length);
int emmc_write(const char *file, int startblock);

// Current 16 KB unit of a running SPI operation, or -1 when none is running.
int spiGetBlocks();
// Flash configuration word read by the most recent spi() call.
int spiGetConfig();
// Last eMMC LBA touched by emmc_read / emmc_write.
unsigned int emmcGetBlocks();
// Request a running SPI operation to stop; spi() then returns -1.
void spiStop();
}
