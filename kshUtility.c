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
		if (GetLastError() == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(o.hEvent, timeout) == WAIT_OBJECT_0)
				success = true;
		}

		GetOverlappedResult(handle, &o, dwRead, FALSE);
	}
	else
		success = true;

	CloseHandle(o.hEvent);

	return success;
}

void checkSumCalculate(FILE* fp, int8_t* data)
{
	int8_t temp[0x1000] = { 0 }, checkSum = 0xEF;

	for (uint32_t j = 0; feof(fp) == 0; j++)
	{
		fread(temp, 1, 0x1000, fp);

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

void ReadPacket(HANDLE hRead)
{
	DWORD dwRead = 0;
	BYTE* readCommand = (BYTE*)calloc(0x2000,1);

	if (readCommand == NULL)
	{
		printf("memory allocate error");
		exit(1);
	}

	if (readData(hRead, readCommand, 0x2000, &dwRead, 100) == true)
	{
		for (DWORD i = 0; i < dwRead; i++)
		{
			printf("%02x ", readCommand[i]);

			if (i % 8 == 7)
				printf("\n");
		}

		printf("\n");
	}

	free(readCommand);
}