#include "kshUtility.h"

void dataBufWrite(uint8_t* dataBuf, uint32_t bufSize, uint32_t* inputData)
{
	int k = 0;

	for (uint32_t i = 0; i < bufSize; i++)
	{
		if (i != 0 && (i % 4) == 0)
			k++;

		dataBuf[i] = (*(inputData + k) >> (8 * i)) & 0xFF;
	}
}

bool writeData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten)
{
	bool success = false;
	OVERLAPPED o = { 0 };

	o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!WriteFile(handle, (LPCVOID)data, length, dwWritten, &o))
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(o.hEvent, INFINITE) == WAIT_OBJECT_0)
			{
				if (GetOverlappedResult(handle, &o, dwWritten, FALSE))
					success = true;
			}
		}
	}
	else
		success = true;

	if (*dwWritten != length)
		success = false;

	CloseHandle(o.hEvent);

	return success;
}

bool readData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwRead, UINT timeout)
{
	bool success = false;
	OVERLAPPED o = { 0 };

	o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!ReadFile(handle, data, length, dwRead, &o))
	{
		DWORD result = GetLastError();
		if (result == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(o.hEvent, timeout) == WAIT_OBJECT_0)
			{
				success = true;
			}
		}

		else
			printf("Last Error: %d", result);

		GetOverlappedResult(handle, &o, dwRead, FALSE);
		printf("dwRead: %#02x\n", *dwRead);
	}

	else
	{
		printf("dwRead: %#x\n", dwRead[0]);
		success = true;
	}

	CloseHandle(o.hEvent);
	return success;
}

void checkSumCalculate(FILE* fp, int8_t* data, uint32_t onePacketSize)
{
	BYTE* temp = (BYTE*)calloc(onePacketSize, 1);
	int8_t checkSum = 0xEF;

	for (uint32_t j = 0; feof(fp) == 0; j++)
	{
		fread(temp, onePacketSize, 1, fp);

		for (uint32_t i = 0; i < 0x1000; i++)
		{
			checkSum ^= temp[i];
		}

		data[j] = checkSum;
		checkSum = 0xEF;
	}
}

#pragma warning(push)
#pragma warning(disable: 6031; disable: 6011)
void inputComPort(wchar_t* COMPortMask)
{
	wchar_t COMPortNumber[10];
	printf("COM port name(ex: COM5): ");
	wscanf(L"%s", COMPortNumber);

	if (COMPortNumber == NULL)
	{
		printf("COM port number error");
		exit(1);
	}

	wcscat(COMPortMask, COMPortNumber);
}

void inputFileName(char* fileName)
{
	printf("file Name(ex: ksh.bin): ");
	scanf("%s", fileName);
}

int readPacket(HANDLE hRead)
{
	DWORD dwRead = 0;
	BYTE readCommand[0x100] = { 0 };

	if (readCommand == NULL)
	{
		printf("memory allocate error");
		exit(1);
	}

	while (readData(hRead, readCommand, 12, &dwRead, 200) != true);

	for (DWORD i = 0; i < dwRead; i++)
	{
		printf("%02x ", readCommand[i]);
	}

	printf("\n");

	if (readCommand[9] == 0)
		return 1;

	else
		return 0;
}

/*
void addNode(node* target, BYTE data)
{
	node* newNode = (node*)malloc(sizeof(node));

	newNode->next = target->next;
	newNode->data = data;

	target->next = newNode;
}

#pragma warning(pop)

void freeNodeAll(node* head)
{
	while (head != NULL)
	{
		struct NODE* next = head->next;
		free(head);
		head = next;
	}
}

void removeNextNode(node* target)
{
	node* removeNode = target->next;
	target->next = removeNode->next;

	free(removeNode);
}
*/