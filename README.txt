mvSCSI for Windows NT/2K/XP
Copyright (c) 2006, Martin Vorbrodt
-----------------------------------
Version: 1.2
Date:    03-28-2006

SCSI FAQ: http://scsifaq.sitemux.net/



Overview
--------

mvSCSI is a simple C API for programming SCSI devices under Windows NT/2K/XP.
It was developed using C and C++, and inspired by simplicity of ASPI.
It represents my (hopefully successful) effort to make SCSI programming
somewhat easier for the average Joes like myself :)

In order to compile, you will need Boost 1.33.1 and Windows Server 2K3 SP1 DDK.
All structures must be packed (byte alignment), or else... world of undefined.
mvSCSI is now thread safe: internal mutex is used to synchronize all function
calls, see source/mvscsi.cpp. mvSCSI also supports asynchronous execution of
SCSI commands. Notification is done through Win32 events, callback functions,
or variable polling. See source/mvscsitest.cpp for example of each technique.

Enjoy!



mvSCSI Error Codes
------------------

mvSCSI_OK      - Returned by all mvSCSI functions. Indicates
                 a successfully completion of a function.

mvSCSI_NO_INIT - Returned by all mvSCSI functions except mvSCSI_Init.
                 Indicates that mvSCSI has not been initialized yet.
                 Call mvSCSI_Init to initialize mvSCSI.

mvSCSI_ERROR   - Returned by mvSCSI_Init, mvSCSI_RescanBus, mvSCSI_ExecCmd.
                 Indicates internal system error (SPTI, memory allocation, etc).

mvSCSI_CHECK   - Returned by mvSCSI_ExecCmd. Indicates CHECK CONDITION
                 status was returned from SCSI device and/or controller.

mvSCSI_PENDING - Returned by mvSCSI_ExecCmd. Indicates that SCSI request
                 has not completed yet.

mvSCSI_NO_BUS  - Returned by mvSCSI_Init, mvSCSI_RescanBus, and mvSCSI_InquiryBus.
                 Indicates that the specified SCSI bus does not exist or that
                 no SCSI buses were detected during initialization.

mvSCSI_NO_DEV  - Returned by mvSCSI_InquiryDev, mvSCSI_SetDevTimeOut,
                 mvSCSI_ExecCmd. Indicates that the specified SCSI device
                 does not exist.



mvSCSI Predefined Values
------------------------

mvSCSI_MAX_BUS          - Maximum number of SCSI buses.
mvSCSI_MAX_PATH         - Maximum number of SCSI paths on a bus.
mvSCSI_MAX_TARGET       - Maximum number of SCSI targets on a bus.
mvSCSI_MAX_LUN          - Maximum number of SCSI luns on a target.

mvSCSI_MAX_CDB_LEN      - Maxumum length of CDB structure.
mvSCSI_MAX_SENSE_LEN    - Maximum length of sense data.

mvSCSI_EVENT_NOTIFY     - Use Win32 event notification.
mvSCSI_POSTING          - Use callback for notification.
mvSCSI_POLLING          - Use variable polling for notification.

mvSCSI_DATA_OUT         - Data direction from controller to target.
mvSCSI_DATA_IN          - Data direction from target to controller.
mvSCSI_DATA_UNSPECIFIED - Data direction is unspecified.

mvSCSI_DEFAULT_TIMEOUT  - Default timeout value for a SCSI device.

mvSCSI_DASD             - Direct-access device.
mvSCSI_SEQD             - Sequential-access device.
mvSCSI_PRNT             - Printer device.
mvSCSI_PROC             - Processor device.
mvSCSI_WORM             - Write-once device.
mvSCSI_CDROM            - CD-ROM device.
mvSCSI_SCAN             - Scanner device.
mvSCSI_OPTI             - Optical memory device.
mvSCSI_JUKE             - Medium changer device.
mvSCSI_COMM             - Communication device.
mvSCSI_UNKNOWN          - Unknown device.



mvSCSI Structures
-----------------

mvSCSI_Bus structure is used for bus rescan and bus inquiry.
Input fields:
    busNumber - Bus number (starting from 0).
Output fields:
    busMaxTransfer   - Maximum transfer length (in bytes).
    busAlignmentMask - Alignment mask for the data buffer.

    typedef struct {
        BYTE  busNumber;
        DWORD busMaxTransfer;
        DWORD busAlignmentMask;
    } mvSCSI_Bus, *PmvSCSI_Bus, FAR *LPmvSCSI_Bus;

mvSCSI_Dev structure is used for device inquiry and setting device timeout.
Input fields:
    devBus    - Bus part of the device address.
    devPath   - Path part of the device address.
    devTarget - Target part of the device address.
    devLun    - Lun part of the device address.
Output fields:
    devType - Type of the SCSI device.
Input/Output fields:
    devTimeOut - Timeout value for the device.

    typedef struct {
        BYTE  devBus;
        BYTE  devPath;
        BYTE  devTarget;
        BYTE  devLun;
        BYTE  devType;
        DWORD devTimeOut;
    } mvSCSI_Dev, *PmvSCSI_Dev, FAR *LPmvSCSI_Dev;

mvSCSI_Cmd structure is used for executing a SCSI command.
Input fields:
    devBus      - Bus part of the device address.
    devPath     - Path part of the device address.
    devTarget   - Target part of the device address.
    devLun      - Lun part of the device address.
    cmdCDBLen   - Length of CDB.
    cmdCDB      - Array, bytes of CDB.
    cmdSenseLen - Length of sense data.
    cmdDataDir  - Data direction.
    cmdBufLen   - Length of data buffer.
    cmdBufPtr   - Pointer to data buffer.
    cmdPostFlag - Type of notification to use (see "mvSCSI Predefined Values").
    cmdPostProc - Address of callback OR address of EVENT to be signaled.
                  Callback function signature: void (*)(LPmvSCSI_Cmd).
Output fields:
    cmdSense      - Array, bytes of sense data.
    cmdStatus     - mvSCSI status code.
    cmdSCSIStatus - SCSI status code.

    typedef struct {
        BYTE   devBus;
        BYTE   devPath;
        BYTE   devTarget;
        BYTE   devLun;
        BYTE   cmdCDBLen;
        BYTE   cmdCDB[mvSCSI_MAX_CDB_LEN];
        BYTE   cmdSenseLen;
        BYTE   cmdSense[mvSCSI_MAX_SENSE_LEN];
        BYTE   cmdDataDir;
        DWORD  cmdBufLen;
        LPVOID cmdBufPtr;
        BYTE   cmdPostFlag;
        LPVOID cmdPostProc;
        BYTE   cmdStatus;
        BYTE   cmdSCSIStatus;
    } mvSCSI_Cmd, *PmvSCSI_Cmd, FAR *LPmvSCSI_Cmd;



mvSCSI Functions
----------------

DWORD mvSCSI_Init(VOID) - Initializes mvSCSI. Returns 32bit DWORD. Bits 7-0
hold number of SCSI buses detected. Bits 15-8 hold the status code.

DWORD mvSCSI_RescanBus(LPmvSCSI_Bus) - Rescans a given SCSI bus.

DWORD mvSCSI_InquiryBus(LPmvSCSI_Bus) - Inquiries a given SCSI bus.

DWORD mvSCSI_InquiryDev(LPmvSCSI_Dev) - Inquiries a given SCSI device.

DWORD mvSCSI_SetDevTimeOut(LPmvSCSI_Dev) - Sets SCSI device timeout.

DWORD mvSCSI_ExecCmd(LPmvSCSI_Cmd) - Executes a SCSI command on a device.
                                     This function uses asynchronous execution.



mvSCSI Files
------------

source/mvscsi.h   - Main header file, include with all your projects.
source/mvscsi.cpp - Main implementation file, puts SPTI wrappers to some use.

source/mvspti.[h/cpp]       - C++ wrapper around SPTI.
source/mvsptihandle.[h/cpp] - C++ wrapper around HANDLE.

source/mvmutex.h - C++ wrapper around Win32 mutex.

build/mvscsi.exp - Export library for mvSCSI.
build/mvscsi.lib - Import library for mvSCSI. Linking with it implicitly
                   loads mvSCSI.dll and obtains function addresses.
build/mvscsi.dll - The core of the mvSCSI functionality.

source/mvscsitest.cpp - Simple example program source.
source/mvscsitest.exe - Simple example program.



Questions, Comments, Great Thoughts
-----------------------------------

Any feedback would be greatly appreciated: mvorbrodt@gmail.com
If you find mvSCSI useful, please be so kind and let me know.
Also, please email me any suggestions you might have for
improvements. Bug reports (if you find any) would be great too :)
