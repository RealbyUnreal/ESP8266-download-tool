#pragma warning(disable: 6054; disable: 6031)

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ESP8266_SW_ROM_command.h"

//test variable
BYTE Sync[46] = 
{
	0xC0, 0x00, 0x08, 0x24, 0x00,
	0x78, 0x01, 0x3A, 0x00, 0x07,
	0x07, 0x12, 0x20, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55, 0x55,
	0xC0
};

BYTE readReg[14] =
{
	0xC0, 0x00, 0x0A, 0x04, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x00, 0x40, 0xC0
};
//

int main()
{
	wchar_t COMPortNumber[10], COMPortMask[20] = L"\\\\.\\";
	
	printf("COM port name(ex: COM5): ");
	
	wscanf(L"%s", COMPortNumber);

	if (COMPortNumber == NULL)
	{
		printf("COM port number error");
		exit(1);
	}

	wcscat(COMPortMask, COMPortNumber);

	HANDLE hMasterCOM = CreateFile(COMPortMask, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
	PurgeComm(hMasterCOM, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	DCB dcbMasterInitState;
	GetCommState(hMasterCOM, &dcbMasterInitState);

	DCB dcbMaster = dcbMasterInitState;

	dcbMaster.BaudRate = CBR_57600;
	dcbMaster.Parity = NOPARITY;
	dcbMaster.ByteSize = 8;
	dcbMaster.StopBits = ONESTOPBIT;

	SetCommState(hMasterCOM, &dcbMaster);
	Sleep(60);

	//test code
	BYTE testData[109];
	DWORD dwWritten[120] = { 0 };
		
	if (writeData(hMasterCOM, Sync, 46, dwWritten) == true)
	{
		if (writeData(hMasterCOM, Sync, 46, dwWritten) == true)
		{
			if (writeData(hMasterCOM, readReg, 14, dwWritten) == true)
			{
				if (readData(hMasterCOM, testData, 108, dwWritten, 10000) == true)
				{
					for (int i = 0; i < 108; i++)
					{
						printf("%2x\t", testData[i]);
					}
				}
			}
		}
	}
	//

	CloseHandle(hMasterCOM);
	hMasterCOM = INVALID_HANDLE_VALUE;

	return 0;
}