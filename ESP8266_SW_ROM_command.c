#include "ESP8266_SW_ROM_command.h"

#pragma warning(push)
#pragma warning(disable: 4133)
void flashBegin(HANDLE handle, DWORD* dwWritten)
{
	beginWriteCmdType cmd;
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

	writeData(handle, &cmd, sizeof(cmd), dwWritten);
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4996; disable: 6031)
void flashBeginMsg(uint32_t* eraseSize, uint32_t* dataPacketCount, uint32_t* onePacketSize, uint32_t* flashOffset)
{
	printf("Erase size(decimal): ");
	scanf("%d", eraseSize);
	printf("Number of data packets(decimal): ");
	scanf("%d", eraseSize);
	printf("size of one packet(decimal): ");
	scanf("%d", eraseSize);
	printf("flash offset(hex): ");
	scanf("%x", flashOffset);
}
#pragma warning(pop)