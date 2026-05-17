#pragma once

#ifdef _WIN32
    #include <tchar.h>
    #include <windows.h>
#else
    #include "libmpsse/libftd2xx/WinTypes.h"
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <cstring>
    
    // Define Sleep in terms of usleep
    #define Sleep(ms) usleep((ms) * 1000)
    
    // Define empty __declspec for Linux
    #ifndef __declspec
        #define __declspec(x)
    #endif
    
    // Define WINAPI as empty
    #ifndef WINAPI
        #define WINAPI
    #endif

    // Define _stricmp as strcasecmp
    #include <strings.h>
    #define _stricmp strcasecmp
    #define strcpy_s(dest, size, src) strncpy(dest, src, size)
    #define strcat_s(dest, size, src) strncat(dest, src, size)
    #define sprintf_s(buf, size, fmt, ...) snprintf(buf, size, fmt, ##__VA_ARGS__)

    // FTC types for compatibility
    typedef DWORD FTC_HANDLE;
    typedef ULONG FTC_STATUS;
    #define FTC_SUCCESS 0
    // D2XX function redirections to use libmpsse's internal pointers
    #include "libmpsse/source/ftdi_infra.h"
    #define FT_Read varFunctionPtrLst.p_FT_Read
    #define FT_Write varFunctionPtrLst.p_FT_Write
    #define FT_SetBitMode varFunctionPtrLst.p_FT_SetBitmode
    #define FT_Purge varFunctionPtrLst.p_FT_Purge
    #define FT_SetUSBParameters varFunctionPtrLst.p_FT_SetUSBParameters
    #define FT_SetLatencyTimer varFunctionPtrLst.p_FT_SetLatencyTimer
    #define FT_Open varFunctionPtrLst.p_FT_Open
    #define FT_Close varFunctionPtrLst.p_FT_Close
    #define FT_CreateDeviceInfoList varFunctionPtrLst.p_FT_GetNumChannel
    #define FT_GetDeviceInfoList varFunctionPtrLst.p_FT_GetDeviceInfoList

#endif
