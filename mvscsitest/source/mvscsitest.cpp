#include <windows.h>
#include "../../mvscsi/source/mvscsi.h"

#include <iostream>
using namespace std;

#ifdef __BORLANDC__
#pragma option -a1
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct {
	enum { OPCODE = 0x12 };
	BYTE opCode    : 8;
	BYTE evpd      : 1;
	BYTE cmddt     : 1;
	BYTE reserved0 : 3;
	BYTE lun       : 3;
	BYTE pageCode  : 8;
	BYTE reserved1 : 8;
	BYTE length    : 8;
	BYTE control   : 8;
} InquiryCDB;

typedef struct {
	BYTE deviceClass         : 5;
	BYTE peripheralQualifier : 3;
	BYTE reserved0           : 7;
	BYTE rmb                 : 1;
	BYTE ansi                : 3;
	BYTE ecma                : 3;
	BYTE iso                 : 2;
	BYTE dataFormat          : 4;
	BYTE reserved1           : 2;
	BYTE tio                 : 1;
	BYTE aen                 : 1;
	BYTE additionalLength    : 8;
	BYTE reserved2           : 8;
	BYTE reserved3           : 8;
	BYTE sftr                : 1;
	BYTE que                 : 1;
	BYTE reserved4           : 1;
	BYTE link                : 1;
	BYTE sync                : 1;
	BYTE w16                 : 1;
	BYTE w32                 : 1;
	BYTE rel                 : 1;
	BYTE manufacturer[8];
	BYTE product[16];
	BYTE revision[4];
} InquiryData;

/* ******************* */
/* * POSTING EXAMPLE * */
/* ******************* */
HANDLE callbackEvent;
void mvSCSI_Callback(LPmvSCSI_Cmd cmd) {
	SetEvent(callbackEvent);
}
/* ******************* */

int main() {
	DWORD status         = mvSCSI_Init();
	BYTE  mvSCSIStatus   = HIBYTE(LOWORD(status));
	BYTE  mvSCSIBusCount = LOBYTE(LOWORD(status));

	cout << endl;
	cout << "mvSCSI Status: " << int(mvSCSIStatus)   << endl;
	cout << "mvSCSI Buses : " << int(mvSCSIBusCount) << endl << endl << endl;

	for(int busIndex = 0; busIndex < mvSCSIBusCount; ++busIndex) {
		mvSCSI_Bus bus = {0};
		bus.busNumber = busIndex;

		mvSCSI_RescanBus(&bus);

		if(mvSCSI_InquiryBus(&bus) == mvSCSI_OK) {
			cout << "Bus Number       : " << int(bus.busNumber) << endl;
			cout << "Bus Max Transfer : " << int(bus.busMaxTransfer) << endl;
			cout << "Bus Mask         : " << int(bus.busAlignmentMask) << endl << endl;

			for(int devPath = 0; devPath < mvSCSI_MAX_PATH; ++devPath) {
				for(int devTarget = 0; devTarget < mvSCSI_MAX_TARGET; ++devTarget) {
					for(int devLun = 0; devLun < mvSCSI_MAX_LUN; ++devLun) {
						mvSCSI_Dev dev = {0};
						dev.devBus     = busIndex;
						dev.devPath    = devPath;
						dev.devTarget  = devTarget;
						dev.devLun     = devLun;
						dev.devTimeOut = 30;

						if(mvSCSI_SetDevTimeOut(&dev) == mvSCSI_OK && mvSCSI_InquiryDev(&dev) == mvSCSI_OK) {
							cout << "\tDevice at " << int(dev.devBus) << ":" << int(dev.devPath) << ":" << int(dev.devTarget) << ":" << int(dev.devLun) << ", type id: " << int(dev.devType) << ", timeout: " << int(dev.devTimeOut) << endl;

							mvSCSI_Cmd  cmd     = {0};
							InquiryCDB  inqCDB  = {0};
							InquiryData inqData = {0};

							inqCDB.opCode = InquiryCDB::OPCODE;
							inqCDB.length = sizeof(inqData);

							cmd.devBus      = busIndex;
							cmd.devPath     = devPath;
							cmd.devTarget   = devTarget;
							cmd.devLun      = devLun;
							cmd.cmdCDBLen   = sizeof(inqCDB);
							memcpy(cmd.cmdCDB, &inqCDB, sizeof(inqCDB));
							cmd.cmdSenseLen = mvSCSI_MAX_SENSE_LEN;
							cmd.cmdDataDir  = mvSCSI_DATA_IN;
							cmd.cmdBufLen   = sizeof(inqData);
							cmd.cmdBufPtr   = &inqData;

							/* ************************ */
							/* * EVENT NOTIFY EXAMPLE * */
							/* ************************ */
							HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);
							ResetEvent(event);

							cmd.cmdPostFlag = mvSCSI_EVENT_NOTIFY;
							cmd.cmdPostProc = (LPVOID)event;

							if(mvSCSI_ExecCmd(&cmd) == mvSCSI_PENDING) {
								WaitForSingleObject(event, INFINITE);
								CloseHandle(event);
							}
							/* ************************ */

							/* ******************* */
							/* * POSTING EXAMPLE * */
							/* ******************* *
							callbackEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
							ResetEvent(callbackEvent);

							cmd.cmdPostFlag = mvSCSI_POSTING;
							cmd.cmdPostProc = mvSCSI_Callback;
							mvSCSI_ExecCmd(&cmd);

							WaitForSingleObject(callbackEvent, INFINITE);
							CloseHandle(callbackEvent);
							/* ************************ */

							/* ******************* */
							/* * POLLING EXAMPLE * */
							/* ******************* *
							cmd.cmdPostFlag = mvSCSI_POLLING;
							mvSCSI_ExecCmd(&cmd);
							while(cmd.cmdStatus == mvSCSI_PENDING) { Sleep(1); }
							/* ************************ */

							if(cmd.cmdStatus == mvSCSI_OK) {
								cout << "\t[" << inqData.manufacturer << "]" << endl << endl;
							}
						}
					}
				}
			}
			cout << endl;
		}
	}

	return 0;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif
