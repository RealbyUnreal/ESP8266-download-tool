#include "ESP8266_SW_ROM_command.h"

#pragma warning(push)
#pragma warning(disable: 4133)
void flashBegin(HANDLE handle, DWORD* dwWritten, int j)
{
	beginWriteCmdType cmd;
	BYTE* cp = &cmd;
	BYTE startEndSignal = 0xC0;
	BYTE temp[42], tempIndex = 1;

	//uint32_t eraseSize, dataPacketCount, onePacketSize, flashOffset;
	//memFlashBeginMsg(&eraseSize, &dataPacketCount, &onePacketSize, &flashOffset, ESP_FLASH_BEGIN);

	uint32_t inputData[4] = { 0x1000 , 0x0001, 0x1000, 0x00 };
	inputData[3] = 0 + (j * 0x1000);
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
	for (int i = 1; i < 25; i++)
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

	temp[tempIndex] = startEndSignal;
	tempIndex++;
	
	if (writeData(handle, temp, tempIndex, dwWritten) == false)
	{
		printf("flash_begin write error");
		exit(1);
	}
}

void memBegin(HANDLE handle, DWORD* dwWritten, int j)
{
	beginWriteCmdType cmd;
	BYTE* cp = &cmd;
	BYTE startEndSignal = 0xC0, IsTrue = 2;
	BYTE temp[42], tempIndex = 1;

	//uint32_t eraseSize, dataPacketCount, onePacketSize, flashOffset;
	//memFlashBeginMsg(&eraseSize, &dataPacketCount, &onePacketSize, &flashOffset, ESP_FLASH_BEGIN);

	uint32_t inputData[4] = { 0x1000 , 0x0001, 0x1000, (j * 0x1000) };

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
	for (int i = 1; i < 25; i++)
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

	temp[tempIndex] = startEndSignal;
	tempIndex++;
	while (IsTrue == 1)
	{
		if (writeData(handle, temp, tempIndex, dwWritten) == false)
		{
			printf("flash_begin write error");
			exit(1);
		}

		IsTrue = readPacket(handle);
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
			printf("Erase size(hex, ex:0x201000): ");
			scanf("%x", eraseSize);
			printf("Number of data packets(hex, ex:0x201): ");
			scanf("%x", dataPacketCount);
			printf("size of one packet(hex, ex:0x1000): ");
			scanf("%x", onePacketSize);
			printf("flash offset(hex, ex:0x40200000): ");
			scanf("%x", flashOffset);
			break;
		
		case ESP_MEM_BEGIN:
			printf("total size(hex, ex:0x201000): ");
			scanf("%x", eraseSize);
			printf("Number of data packets(hex, ex:0x201): ");
			scanf("%x", dataPacketCount);
			printf("size of one packet(hex, ex:0x1000): ");
			scanf("%x", onePacketSize);
			printf("memory address(hex, ex:0x40000000): ");
			scanf("%x", flashOffset);
			break;
	}
}
#pragma warning(pop)

void flashDataByFile(HANDLE handle, DWORD* dwWritten, FILE* fp, uint8_t CheckSum)
{
	uint8_t temp[0x2000] = { 0 };
	beginWriteCmdType cmd;
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t tempIndex = 0;
	uint32_t inputData[4] = { 0x1000, 0x00, 0x00,0x00 }; //"Data to write" length,Sequence number, 0, 0
	
	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = ESP_FLASH_DATA;
	cmd.size[0] = 0x00;
	cmd.size[1] = 0x10;
	cmd.checkSum[1] = 0x00;
	cmd.checkSum[2] = 0x00;
	cmd.checkSum[3] = 0x00;

	tempIndex = 1;
	cmd.checkSum[0] = (CheckSum & 0xFF);
	inputData[1] = 0x00;
	dataBufWrite(cmd.dataBuffer, 16, inputData);
	temp[0] = startEndSignal;

	for (uint32_t i = 1; i < 0x1000 + 25; i++)
	{
		if (i < 25)
		{
			temp[tempIndex] = cp[i];
			tempIndex++;
		}
		else if (i >= 25 && i < 0x1000 + 25)
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

	while (writeData(handle, temp, tempIndex, dwWritten) == false);
}

void memDataByFile(HANDLE handle, DWORD* dwWritten, FILE* fp, uint8_t* CheckSum)
{
	uint32_t count = 0x1000 + 26;
	uint8_t temp[0x2000];
	beginWriteCmdType cmd;
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t tempIndex, countGap = 0;
	uint32_t inputData[4] = { 0x1000, 0x00, 0x00,0x00 }; //"Data to write", length,Sequence number, 0, 0

	cmd.direction = 0x00;
	cmd.order = ESP_MEM_DATA;
	cmd.size[0] = 0x00;
	cmd.size[1] = 0x10;
	cmd.checkSum[1] = 0x00;
	cmd.checkSum[2] = 0x00;
	cmd.checkSum[3] = 0x00;

	for (int j = 0; feof(fp) == 0; j++)
	{
		cmd.checkSum[0] = (CheckSum[j] & 0xFF);
		inputData[1] = true;
		dataBufWrite(cmd.dataBuffer, 16, inputData);

		temp[0] = startEndSignal;
		tempIndex = 1;

		for (uint32_t i = 1; i < 0x1000 + 25; i++)
		{
			if (i < 25)
			{
				temp[tempIndex] = cp[i];
			}

			else if (i >= 25 && i < 0x1000 + 25)
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

		flashBegin(handle, dwWritten, j);
		if (writeData(handle, temp, tempIndex, dwWritten) == false)
		{
			printf("file data write error");
			exit(1);
		}

		//readPacket(handle);
	}
}


