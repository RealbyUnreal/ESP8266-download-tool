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

BYTE FLASH_END[14] =
{
	0xC0, 0x00, 0x04, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0xC0
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
	DWORD dwWritten[2048] = { 0 };
	int8_t checkSum[200] = { 0 };

	char fileName[100] = " ";

	printf("file Name(ex: ksh.bin): ");
	scanf("%s", fileName);
	FILE* fp = fopen((const char*)fileName, "rb");

	if (fp == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}

	checkSumCalculate(fp, checkSum);

	fseek(fp, 0, SEEK_SET);
	Sleep(60);

	if(writeData(hMasterCOM, Sync, 46, dwWritten) == true)
	{
		Sleep(60);
		if (writeData(hMasterCOM, Sync, 46, dwWritten) == true)
		{
			Sleep(60);
			flashBegin(hMasterCOM, dwWritten);
			Sleep(60);
			flashDataByFile(hMasterCOM, dwWritten, fp, checkSum);
		}
	}

	if(writeData(hMasterCOM, FLASH_END, 14, dwWritten) == false)
	{
		printf("flash_ene write error");
		exit(1);
	}

	fclose(fp);
	CloseHandle(hMasterCOM);
	hMasterCOM = INVALID_HANDLE_VALUE;

	return 0;
}

/*
	//test code
	BYTE testData[109];


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
	*/