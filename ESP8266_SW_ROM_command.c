#include "ESP8266_SW_ROM_command.h"

#pragma warning(push)
#pragma warning(disable: 4133)
void flashBegin(HANDLE handle, DWORD* dwWritten)
{
	beginWriteCmdType cmd;
	BYTE* cp = &cmd;
	BYTE startEndSignal = 0xC0;

	uint32_t eraseSize, dataPacketCount, onePacketSize, flashOffset;
	flashBeginMsg(&eraseSize, &dataPacketCount, &onePacketSize, &flashOffset);

	uint32_t inputData[4] = { eraseSize, dataPacketCount, onePacketSize, flashOffset };

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
	//

	for (int i = 1; i < sizeof(cmd); i++)
	{
		switch (cp[i])
		{
			case 0xDB:
				cp[i] = 0xDC;
				break;
			case 0xc0:
				cp[i] = 0xDB;
				break;
			default:
				break;
		}
	}

	if(writeData(handle, &cmd, sizeof(cmd), dwWritten) == false)
	{
		printf("flash_begin write error");
		exit(1);
	}
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4996; disable: 6031)
void flashBeginMsg(uint32_t* eraseSize, uint32_t* dataPacketCount, uint32_t* onePacketSize, uint32_t* flashOffset)
{
	printf("Erase size(hex): ");
	scanf("%x", eraseSize);
	printf("Number of data packets(hex): ");
	scanf("%x", eraseSize);
	printf("size of one packet(hex): ");
	scanf("%x", eraseSize);
	printf("flash offset(hex): ");
	scanf("%x", flashOffset);
}
#pragma warning(pop)

void flashDataByFile(HANDLE handle, DWORD* dwWritten, FILE* fp, uint8_t* checkSum)
{
	uint8_t* temp = (uint8_t*)malloc(0x1FFF);
	uint32_t i = 0;
	beginWriteCmdType cmd;
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t inputData[4] = { 0x1000, 0x00, 0x00,0x00 }; // 0,0,Sequence number,"Data to write" length

	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = ESP_FLASH_DATA;
	cmd.size[0] = 0x00;
	cmd.size[1] = 0x10;
	cmd.checkSum[1] = 0x00;
	cmd.checkSum[2] = 0x00;
	cmd.checkSum[3] = 0x00;
	

	for (int j = 0; j < 0x200; j++)
	{
		inputData[1] = (j * 0x1000);
		cmd.checkSum[0] = checkSum[j];
		dataBufWrite(cmd.dataBuffer, 16, inputData);

		for (i = 0 ; i < 25 ; i++)
		{
			*(temp + i) = cp[i];
		}

		fread(&temp[25], sizeof(uint8_t), 0x1000, fp);

		for (i = 1; i < (0x1000 + 25); i++)
		{
			switch (temp[i])
			{
			case 0xDB:
				temp[i] = 0xDC;
				break;
			case 0xc0:
				temp[i] = 0xDB;
				break;
			default:
				break;
			}
		}

		*(temp + i) = startEndSignal;

		if (writeData(handle, temp, (0x1000 + 26), dwWritten) == false)
		{
			printf("file data write error");
			exit(1);
		}
	}
	free(temp);
}