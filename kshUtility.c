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