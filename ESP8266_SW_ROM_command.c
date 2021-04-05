#include "ESP8266_SW_ROM_command.h"

#pragma warning(push)
#pragma warning(disable: 4133)
void flashBegin(HANDLE handle, DWORD* dwWritten)
{
	beginWriteCmdType cmd;
	BYTE* cp = &cmd;
	BYTE startEndSignal = 0xC0;
	BYTE temp[42], tempIndex = 1;

	uint32_t totalSize, dataPacketCount, onePacketSize, memoryOffset;
	memFlashBeginMsg(&totalSize, &dataPacketCount, &onePacketSize, &memoryOffset, ESP_FLASH_BEGIN);

	uint32_t inputData[4] = { totalSize, dataPacketCount, onePacketSize, memoryOffset };

	//
	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = ESP_FLASH_BEGIN;
	cmd.size[0] = 0x10;
	cmd.size[1] = 0x00;
	for (int i = 0; i < 4; i++)
	{
		cmd.checkSum[i] = 0;
	}
	dataBufWrite(cmd.dataBuffer, 16, inputData);
	cmd.endSignal = startEndSignal;
	// 26bytes

	temp[0] = startEndSignal;
	for (int i = 1; i < 26; i++)
	{
		temp[tempIndex] = cp[i];

		switch ((int)temp[tempIndex])
		{
		case 0xDB:
			temp[tempIndex] = 0xDB;
			tempIndex++;

			temp[tempIndex] = 0xDD;
			break;
		case 0xC0:
			temp[tempIndex] = 0xDB;
			tempIndex++;

			temp[tempIndex] = 0xDC;
			break;
		}

		tempIndex++;
	}

	if (writeData(handle, temp, tempIndex, dwWritten) == false)
	{
		printf("mem_begin write error");
		exit(1);
	}
}

void memBegin(HANDLE handle, DWORD* dwWritten)
{
	beginWriteCmdType cmd;
	BYTE* cp = &cmd;
	BYTE startEndSignal = 0xC0;
	BYTE temp[42], tempIndex = 1;

	uint32_t totalSize, dataPacketCount, onePacketSize, memoryOffset;
	memFlashBeginMsg(&totalSize, &dataPacketCount, &onePacketSize, &memoryOffset, ESP_MEM_BEGIN);

	uint32_t inputData[4] = { totalSize, dataPacketCount, onePacketSize, memoryOffset };

	//
	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = ESP_MEM_BEGIN;
	cmd.size[0] = 0x10;
	cmd.size[1] = 0x00;
	for (int i = 0; i < 4; i++)
	{
		cmd.checkSum[i] = 0;
	}
	dataBufWrite(cmd.dataBuffer, 16, inputData);
	cmd.endSignal = startEndSignal;
	// 26bytes

	temp[0] = startEndSignal;
	for (int i = 1; i < 26; i++)
	{
		temp[tempIndex] = cp[i];

		switch ((int)temp[tempIndex])
		{
		case 0xDB:
			temp[tempIndex] = 0xDB;
			tempIndex++;

			temp[tempIndex] = 0xDD;
			break;
		case 0xC0:
			temp[tempIndex] = 0xDB;
			tempIndex++;

			temp[tempIndex] = 0xDC;
			break;
		}

		tempIndex++;
	}

	if (writeData(handle, temp, tempIndex, dwWritten) == false)
	{
		printf("mem_begin write error");
		exit(1);
	}
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4996; disable: 6031)
void memFlashBeginMsg(uint32_t* eraseSize, uint32_t* dataPacketCount, uint32_t* onePacketSize, uint32_t* flashOffset, int type)
{
	switch (type)
	{
		case ESP_FLASH_BEGIN:
			printf("Erase size(hex): ");
			scanf("%x", eraseSize);
			printf("Number of data packets(hex): ");
			scanf("%x", dataPacketCount);
			printf("size of one packet(hex): ");
			scanf("%x", onePacketSize);
			printf("flash offset(hex): ");
			scanf("%x", flashOffset);
			break;
		
		case ESP_MEM_BEGIN:
			printf("total size(hex): ");
			scanf("%x", eraseSize);
			printf("Number of data packets(hex): ");
			scanf("%x", dataPacketCount);
			printf("size of one packet(hex): ");
			scanf("%x", onePacketSize);
			printf("flash offset(hex): ");
			scanf("%x", flashOffset);
			break;
	}
}
#pragma warning(pop)

void flashDataByFile(HANDLE handle, DWORD* dwWritten, FILE* fp, uint8_t* checkSum)
{
	uint32_t count = 0x1000 + 26;
	uint8_t temp[0x1FFF] = { 0 };
	beginWriteCmdType cmd;
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t tempIndex, countGap = 0;
	uint32_t inputData[4] = { 0x1000, 0x00, 0x00,0x00 }; //"Data to write", length,Sequence number, 0, 0

	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = ESP_FLASH_DATA;
	cmd.size[0] = 0x00;
	cmd.size[1] = 0x10;
	cmd.checkSum[1] = 0x00;
	cmd.checkSum[2] = 0x00;
	cmd.checkSum[3] = 0x00;

	for (int j = 0; feof(fp) == 0; j++)
	{
		tempIndex = 1;
		cmd.checkSum[0] = checkSum[j];
		inputData[1] += (0x1000 + countGap);
		dataBufWrite(cmd.dataBuffer, 16, inputData);
		temp[0] = startEndSignal;	

		for (uint32_t i = 1; i < 0x1000 + 25; i++)
		{
			if (i < 25)
			{
				temp[tempIndex] = cp[i];
			}

			else if (i > 25 && i < 0x1000 + 25)
			{
				fread(&temp[tempIndex], sizeof(uint8_t), 1, fp);
			}

			switch ((int)temp[tempIndex])
			{
				case 0xDB:
					temp[tempIndex] = 0xDB;
					tempIndex++;

					temp[tempIndex] = 0xDD;
					break;
				case 0xC0:
					temp[tempIndex] = 0xDB;
					tempIndex++;

					temp[tempIndex] = 0xDC;
					break;
			}
			tempIndex++;
		}

		temp[tempIndex] = startEndSignal;
		tempIndex++;

		countGap = tempIndex - count;
		if (writeData(handle, temp, tempIndex, dwWritten) == false)
		{
			printf("file data write error");
			exit(1);
		}
	}
}

void memDataByFile(HANDLE handle, DWORD* dwWritten, FILE* fp, uint8_t* checkSum)
{
	uint32_t count = 0x1000 + 26;
	uint8_t temp[0x1FFF] = { 0 };
	beginWriteCmdType cmd;
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t tempIndex, countGap = 0;
	uint32_t inputData[4] = { 0x1000, 0x00, 0x00,0x00 }; //"Data to write", length,Sequence number, 0, 0

	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = ESP_MEM_DATA;
	cmd.size[0] = 0x00;
	cmd.size[1] = 0x10;
	cmd.checkSum[1] = 0x00;
	cmd.checkSum[2] = 0x00;
	cmd.checkSum[3] = 0x00;

	for (int j = 0; feof(fp) == 0; j++)
	{
		tempIndex = 1;
		cmd.checkSum[0] = checkSum[j];
		inputData[1] += (0x1000 + countGap);
		dataBufWrite(cmd.dataBuffer, 16, inputData);
		temp[0] = startEndSignal;

		for (uint32_t i = 1; i < 0x1000 + 25; i++)
		{
			if (i < 25)
			{
				temp[tempIndex] = cp[i];
			}

			else if (i > 25 && i < 0x1000 + 25)
			{
				fread(&temp[tempIndex], sizeof(uint8_t), 1, fp);
			}

			switch ((int)temp[tempIndex])
			{
			case 0xDB:
				temp[tempIndex] = 0xDB;
				tempIndex++;

				temp[tempIndex] = 0xDD;
				break;
			case 0xC0:
				temp[tempIndex] = 0xDB;
				tempIndex++;

				temp[tempIndex] = 0xDC;
				break;
			}
			tempIndex++;
		}

		temp[tempIndex] = startEndSignal;
		tempIndex++;

		countGap = tempIndex - count;
		if (writeData(handle, temp, tempIndex, dwWritten) == false)
		{
			printf("file data write error");
			exit(1);
		}
	}
}


