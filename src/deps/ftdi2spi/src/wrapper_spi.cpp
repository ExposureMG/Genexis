#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Include libmpsse headers
#include "libmpsse/libftd2xx/ftd2xx.h"
#include "libmpsse/include/libmpsse_spi.h"

// Use the cross-platform types provided by libmpsse/ftd2xx
#include "libmpsse/libftd2xx/WinTypes.h"

#include "wrapper_spi.h"

#define MAX_NUM_BYTES_USB_WRITE 16384
#define MAX_READ_DATA_WORDS_BUFFER_SIZE 65536
#define MAX_FREQ_CLOCK_DIVISOR 0

#define CHIP_SELECT_PIN 0x08

// MPSSE Commands
const uint8_t SET_LOW_BYTE_DATA_BITS_CMD = 0x80;
const uint8_t SET_HIGH_BYTE_DATA_BITS_CMD = 0x82;
const uint8_t SEND_ANSWER_BACK_IMMEDIATELY_CMD = 0x87;

const uint8_t CLK_DATA_BYTES_OUT_ON_NEG_CLK_LSB_FIRST_CMD = 0x19;
const uint8_t CLK_DATA_BITS_OUT_ON_NEG_CLK_LSB_FIRST_CMD = 0x1B;
const uint8_t CLK_DATA_BYTES_IN_ON_NEG_CLK_LSB_FIRST_CMD = 0x2D;
const uint8_t CLK_DATA_BITS_IN_ON_NEG_CLK_LSB_FIRST_CMD = 0x2F;

FT_HANDLE ftHandle = NULL;
BYTE byOutputBuffer[65535];
BYTE dwLowPinsValue = 0x08; // Start with CS High
DWORD dwNumBytesToSend = 0;
DWORD dwNumBytesSent = 0;
DWORD dwNumBytesToRead = 0;

bool spi_init(void) {
    FT_STATUS status;
    DWORD numChannels;
    
    // Initialize libmpsse
    Init_libMPSSE();
    
    status = SPI_GetNumChannels(&numChannels);
    if (status != FT_OK || numChannels == 0) {
        printf("No FTDI channels found\n");
        return false;
    }
    
    // Find Channel B (usually index 1 for dual port devices)
    DWORD channelIndex = 0;
    FT_DEVICE_LIST_INFO_NODE chanInfo;
    for (DWORD i = 0; i < numChannels; i++) {
        SPI_GetChannelInfo(i, &chanInfo);
        // On xFlasher/NAND-X, Channel B is typically used for SPI
        if (strstr(chanInfo.Description, " B") || strstr(chanInfo.Description, "-B")) {
            channelIndex = i;
            break;
        }
    }
    
    status = SPI_OpenChannel(channelIndex, &ftHandle);
    if (status != FT_OK) {
        printf("Failed to open SPI channel\n");
        return false;
    }
    
    ChannelConfig config;
    memset(&config, 0, sizeof(config));
    config.ClockRate = 30000000; // 30MHz (Matches divisor 0 on HS devices)
    config.LatencyTimer = 1;
    config.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    
    // Pin configuration (Value High, Dir High, Value Low, Dir Low)
    // Low Byte: SK=1, DO=1, DI=in, CS=1, GPIOL0-1=out -> 0xFB Dir, 0x3B Value
    // High Byte: ACBUS0=out, ACBUS1=out -> 0x03 Dir, 0x03 Value
    config.Pin = 0x03033BFB; 
    
    status = SPI_InitChannel(ftHandle, &config);
    if (status != FT_OK) {
        printf("Failed to init SPI channel\n");
        SPI_CloseChannel(ftHandle);
        ftHandle = NULL;
        return false;
    }
    
    dwLowPinsValue = 0x3B; // Initial state matching config.Pin low byte value
    
    return true;
}

void SendBytesToDevice(void) {
    if (dwNumBytesToSend == 0) return;
    
    FT_STATUS status;
    DWORD bytesWritten;
    
    // We use raw FT_Write because FTDI2SPI builds custom MPSSE command streams
    status = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &bytesWritten);
    
    if (status != FT_OK) {
        printf("USB Write Failed: %d\n", (int)status);
    }
    
    dwNumBytesToSend = 0;
}

void ClearOutputBuffer(void) {
    dwNumBytesToSend = 0;
}

void AddByteToOutputBuffer(BYTE DataByte, bool bClearOutputBuffer) {
    if (bClearOutputBuffer) dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = DataByte;
}

void SetAnswerFast(void) {
    AddByteToOutputBuffer(SEND_ANSWER_BACK_IMMEDIATELY_CMD, false);
}

void GetDataFromDevice(unsigned int numBytesToRead, unsigned char ReadDataBuffer[]) {
    DWORD bytesRead = 0;
    DWORD totalRead = 0;
    int tries = 100;
    
    while (totalRead < numBytesToRead && tries-- > 0) {
        FT_STATUS status = FT_Read(ftHandle, &ReadDataBuffer[totalRead], numBytesToRead - totalRead, &bytesRead);
        if (status != FT_OK) break;
        totalRead += bytesRead;
        if (bytesRead == 0) {
            // Small sleep could be added here if needed
        }
    }
    
    if (totalRead < numBytesToRead) {
        printf("ERROR: NO DATA FROM DEVICE (Read %u/%u)\n", totalRead, numBytesToRead);
        // In a real port, we might want to throw or return error
    }
}

void DisableSPIChip(void) {
    AddByteToOutputBuffer(SET_LOW_BYTE_DATA_BITS_CMD, false);
    dwLowPinsValue |= CHIP_SELECT_PIN; // CS High
    AddByteToOutputBuffer(dwLowPinsValue, false);
    AddByteToOutputBuffer(0xFB, false); // Direction SK, DO, CS, GPIOL1-4 as output
}

void EnableSPIChip(void) {
    AddByteToOutputBuffer(SET_LOW_BYTE_DATA_BITS_CMD, false);
    dwLowPinsValue &= ~CHIP_SELECT_PIN; // CS Low
    AddByteToOutputBuffer(dwLowPinsValue, false);
    AddByteToOutputBuffer(0xFB, false);
}

void AddWriteOutBuffer(DWORD dwNumControlBitsToWrite, unsigned char pWriteControlBuffer[]) {
    if (dwNumControlBitsToWrite <= 1) return;

    DWORD dwModNumControlBitsToWrite = dwNumControlBitsToWrite - 1;
    DWORD dwNumControlBytes = dwModNumControlBitsToWrite / 8;
    DWORD dwControlBufferIndex = 0;

    if (dwNumControlBytes > 0) {
        DWORD bytesToSend = dwNumControlBytes - 1;
        AddByteToOutputBuffer(CLK_DATA_BYTES_OUT_ON_NEG_CLK_LSB_FIRST_CMD, false);
        AddByteToOutputBuffer((BYTE)(bytesToSend & 0xFF), false);
        AddByteToOutputBuffer((BYTE)((bytesToSend >> 8) & 0xFF), false);

        for (DWORD i = 0; i < dwNumControlBytes; i++) {
            AddByteToOutputBuffer(pWriteControlBuffer[dwControlBufferIndex++], false);
        }
    }

    DWORD dwNumRemainingControlBits = dwModNumControlBitsToWrite % 8;
    if (dwNumRemainingControlBits > 0) {
        AddByteToOutputBuffer(CLK_DATA_BITS_OUT_ON_NEG_CLK_LSB_FIRST_CMD, false);
        AddByteToOutputBuffer((BYTE)(dwNumRemainingControlBits & 0xFF), false);
        AddByteToOutputBuffer(pWriteControlBuffer[dwControlBufferIndex], false);
    }
}

void AddReadOutBuffer(DWORD dwNumDataBitsToRead) {
    if (dwNumDataBitsToRead == 0) return;
    
    DWORD dwModNumBitsToRead = dwNumDataBitsToRead - 1;
    DWORD dwNumDataBytes = dwModNumBitsToRead / 8;

    if (dwNumDataBytes > 0) {
        DWORD bytesToRead = dwNumDataBytes - 1;
        AddByteToOutputBuffer(CLK_DATA_BYTES_IN_ON_NEG_CLK_LSB_FIRST_CMD, false);
        AddByteToOutputBuffer((BYTE)(bytesToRead & 0xFF), false);
        AddByteToOutputBuffer((BYTE)((bytesToRead >> 8) & 0xFF), false);
    }

    DWORD dwNumRemainingDataBits = dwModNumBitsToRead % 8;
    if (dwNumRemainingDataBits > 0) {
        AddByteToOutputBuffer(CLK_DATA_BITS_IN_ON_NEG_CLK_LSB_FIRST_CMD, false);
        AddByteToOutputBuffer((BYTE)(dwNumRemainingDataBits & 0xFF), false);
    }
}

void spi_SetCS(bool ChipSelect) {
    dwNumBytesToSend = 0;
    
    AddByteToOutputBuffer(SET_LOW_BYTE_DATA_BITS_CMD, false);
    if (ChipSelect) dwLowPinsValue |= 0x08;
    else dwLowPinsValue &= ~0x08;
    
    AddByteToOutputBuffer(dwLowPinsValue, false);
    AddByteToOutputBuffer(0x3E, false); // Matches Windows version discrepancy
    
    SendBytesToDevice();
}

void spi_setGPIO(bool XXLo, bool EJLo) {
    dwNumBytesToSend = 0;
    
    AddByteToOutputBuffer(SET_LOW_BYTE_DATA_BITS_CMD, false);
    dwLowPinsValue &= ~0x30;
    dwLowPinsValue |= (XXLo ? 0x10 : 0x00) | (EJLo ? 0x20 : 0x00);
    
    AddByteToOutputBuffer(dwLowPinsValue, false);
    AddByteToOutputBuffer(0x3E, false); // Matches Windows version discrepancy
    
    SendBytesToDevice();
}

void closeDevice() {
    if (ftHandle) {
        SPI_CloseChannel(ftHandle);
        ftHandle = NULL;
    }
}