#pragma warning(disable: 6054; disable: 6031)

#define SYNC_SIZE 46

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ESP8266_SW_ROM_command.h"

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
	0xC0, 0x00, 0x04, 0x04, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0xC0
};

BYTE MEM_END[18] =
{
	0xC0, 0x00, 0x06, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xC0
};

int main()
{
	wchar_t COMPortMask[20] = L"\\\\.\\";
	inputComPort(COMPortMask);
	BYTE readCommand[0x1000] = { 0 };

	HANDLE hMasterCOM = CreateFile(COMPortMask, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
	PurgeComm(hMasterCOM, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	DCB dcbMasterInitState;
	GetCommState(hMasterCOM, &dcbMasterInitState);

	DCB dcbMaster = dcbMasterInitState;

	dcbMaster.BaudRate = CBR_115200;
	dcbMaster.Parity = NOPARITY;
	dcbMaster.ByteSize = 8;
	dcbMaster.StopBits = ONESTOPBIT;

	SetCommState(hMasterCOM, &dcbMaster);
	Sleep(60);

	DWORD dwWritten = 0;
	DWORD dwRead = 0;
	BYTE* checkSum = (BYTE*)calloc(0x208, 1);
	bool qq = false;
	int repeat = 0;
	
	if (writeData(hMasterCOM, Sync, SYNC_SIZE, &dwWritten) == false)
	{
		printf("Sync command error");
		exit(1);
	}

	if (writeData(hMasterCOM, Sync, SYNC_SIZE, &dwWritten) == false)
	{
		printf("Sync command error");
		exit(1);
	}

	qq = readData(hMasterCOM, readCommand, 96, &dwRead, 100);

	if (qq == false)
	{
		printf("sync err");
		exit(1);
	}

	for (unsigned int i = 0; i < dwRead; i++)
	{
		printf("%02x\t", readCommand[i]);
	}

	printf("\n");

	const char fileName[MAX_PATH] = "factory_WROOM-02N_portChangedByJ.Min.bin";

	//inputFileName(fileName);
	FILE* fp = fopen(fileName, "rb");

	if (fp == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}

	checkSumCalculate(fp, checkSum);

	fseek(fp, 0, SEEK_SET);

	for (int j = 0; feof(fp) == 0; j++)
	{
		while (repeat == 0)
		{
			flashBegin(hMasterCOM, &dwWritten, j);
			repeat = readPacket(hMasterCOM);

			flashDataByFile(hMasterCOM, &dwWritten, fp, checkSum[j]);
			printf("\ncheckSum = %#02X\n", checkSum[j]);
			while (readPacket(hMasterCOM) != 1);
		}
	}

	while (qq != true)
	{
		if(writeData(hMasterCOM, FLASH_END, 14, &dwWritten) == false)
		{
			printf("flash_end write error");
			exit(1);
		}

		qq = readData(hMasterCOM, readCommand, 2, &dwRead, 100);
	}

	fclose(fp);

	SetCommState(hMasterCOM, &dcbMasterInitState);
	Sleep(60);

	CloseHandle(hMasterCOM);
	hMasterCOM = INVALID_HANDLE_VALUE;
	
	free(checkSum);

	return 0;
}

/*
	if (readData(hMasterCOM, readCommand, 100, &dwRead, 200) == true)
	{
		for (int i = 0; i < 150; i++)
		{
			printf("%02x\t", readCommand[i]);
		}
	}
*/

