#include "ESP8266_SW_ROM_command.h"

#pragma warning(push)
#pragma warning(disable: 4133)
void flashBegin(HANDLE handle, DWORD* dwWritten, int j, uint32_t eraseSize, uint32_t dataPacketCount, uint32_t onePacketSize, uint32_t flashOffset)
{
	beginWriteCmdType cmd = { 0 };
	BYTE* cp = &cmd;
	BYTE startEndSignal = 0xC0;
	BYTE tmp[42] = { 0 }, tmpIndex = 1;

	//uint32_t eraseSize, dataPacketCount, onePacketSize, flashOffset;
	//memFlashBeginMsg(&eraseSize, &dataPacketCount, &onePacketSize, &flashOffset, ESP_FLASH_BEGIN);

	uint32_t inputData[4] = { eraseSize , dataPacketCount, onePacketSize, flashOffset};
	inputData[3] = (j * onePacketSize);
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

	tmp[0] = startEndSignal;
	for (int i = 1; i < 25; i++)
	{
		tmp[tmpIndex] = cp[i];

		switch ((int)tmp[tmpIndex])
		{
		case 0xDB:
			tmp[tmpIndex] = 0xDB;
			tmpIndex++;

			tmp[tmpIndex] = 0xDD;
			break;
		case 0xC0:
			tmp[tmpIndex] = 0xDB;
			tmpIndex++;

			tmp[tmpIndex] = 0xDC;
			break;
		}

		tmpIndex++;
	}

	tmp[tmpIndex] = startEndSignal;
	tmpIndex++;

	if (writeData(handle, tmp, tmpIndex, dwWritten) == false)
	{
		printf("flash_begin write error\n");
	}
}
/*
void memBegin(HANDLE handle, DWORD* dwWritten, int j)
{
	beginWriteCmdType cmd = { 0 };
	BYTE* cp = &cmd;
	BYTE startEndSignal = 0xC0;
	BYTE temp[42] = { 0 }, tempIndex = 1;
	bool IsTrue = false;

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
	while (IsTrue == false)
	{
		IsTrue = writeData(handle, temp, tempIndex, dwWritten);
		readPacket(handle);
	}
}
*/
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4996; disable: 6031)
void memFlashBeginMsg(uint32_t* eraseSize, uint32_t* dataPacketCount, uint32_t* onePacketSize, uint32_t* flashOffset, int type)
{
	switch (type)
	{
	case ESP_FLASH_BEGIN:
		printf("Erase size(hex, ex:0x1000): ");
		scanf("%x", eraseSize);
		printf("Number of data packets == 1\n");
		*onePacketSize = 1;
		printf("size of one packet(hex, ex:0x1000): ");
		scanf("%x", onePacketSize);
		printf("flash offset(hex, ex:0x00): ");
		scanf("%x", flashOffset);
		break;

	case ESP_MEM_BEGIN:
		printf("total size(hex, ex:0x1000): ");
		scanf("%x", eraseSize);
		printf("Number of data packets(hex, ex:0x01): ");
		scanf("%x", dataPacketCount);
		printf("size of one packet(hex, ex:0x1000): ");
		scanf("%x", onePacketSize);
		printf("memory address(hex, ex:0x00): ");
		scanf("%x", flashOffset);
		break;
	}
}
#pragma warning(pop)

void flashDataByFile(HANDLE handle, DWORD* dwWritten, FILE* fp, uint8_t* CheckSum, uint32_t eraseSize, uint32_t dataPacketCount, uint32_t onePacketSize, uint32_t flashOffset)
{
	uint8_t temp[0x2000] = { 0 };
	uint8_t data[0x1000] = { 0 };
	beginWriteCmdType cmd = { 0 };
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t tempIndex = 0;
	uint32_t inputData[4] = { onePacketSize, 0x00, 0x00,0x00 }; //"Data to write" length,Sequence number, 0, 0
	bool IsTrue = 0;

	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = ESP_FLASH_DATA;
	cmd.size[0] = onePacketSize & 0xFF;
	cmd.size[1] = (onePacketSize >> 2) & 0xFF;
	cmd.checkSum[1] = 0x00;
	cmd.checkSum[2] = 0x00;
	cmd.checkSum[3] = 0x00;

	for (int j = 0; feof(fp) == 0; j++)
	{
		flashBegin(handle, dwWritten, j, eraseSize, dataPacketCount, onePacketSize, flashOffset);
		readPacket(handle);

		tempIndex = 1;
		cmd.checkSum[0] = (CheckSum[j] & 0xFF);
		inputData[1] = 0x00;

		dataBufWrite(cmd.dataBuffer, 16, inputData);

		temp[0] = startEndSignal;
		fread(data, sizeof(uint8_t), onePacketSize, fp);

		for (uint32_t i = 1; i < onePacketSize + 25; i++)
		{
			if (i < 25)
			{
				temp[tempIndex] = cp[i];
			}

			else if (i >= 25 && i < 0x1000 + 25)
			{
				temp[tempIndex] = data[i - 25];
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

		IsTrue = writeData(handle, temp, tempIndex, dwWritten);
		readPacket(handle);

		while (IsTrue == false)
		{
			printf("Write data false \n");

			flashBegin(handle, dwWritten, j, eraseSize, dataPacketCount, onePacketSize, flashOffset);
			readPacket(handle);

			IsTrue = writeData(handle, temp, tempIndex, dwWritten);
			readPacket(handle);
		}
	}
}
/*
void memDataByFile(HANDLE handle, DWORD* dwWritten, FILE* fp, uint8_t* CheckSum)
{
	uint32_t count = 0x1000 + 26;
	uint8_t temp[0x2000] = { 0 };
	beginWriteCmdType cmd = { 0 };
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
*/