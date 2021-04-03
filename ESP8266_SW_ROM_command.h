#pragma once
#include "kshUtility.h"
#include <Windows.h>
#include <stdint.h>

typedef enum {
	ESP_FLASH_BEGIN = 0x02,
	ESP_FLASH_DATA = 0x03,
	ESP_FLASH_END = 0x04,
	ESP_MEM_BEGIN = 0x05,
	ESP_MEM_END = 0x06,
	ESP_MEM_DATA = 0x07,
	ESP_SYNC = 0x08,
	ESP_WRITE_REG = 0x09,
	ESP_READ_REG = 0x0a,
} espSwRomCommand;

#pragma pack(push, 1)
typedef struct
{
	uint8_t startSignal;
	uint8_t direction;
	uint8_t order;
	uint8_t size[2];
	uint8_t checkSum[4];
	uint8_t dataBuffer[16];
	uint8_t endSignal;
}beginWriteCmdType;

typedef struct
{
	uint8_t direction;
	uint8_t order;
	uint16_t size;
	int32_t Value;
	uint8_t dataBuffer;
}commandResponseType;
#pragma pack(pop)

void flashBegin(HANDLE handle, DWORD* dwWritten);
void flashBeginMsg(uint32_t* eraseSize, uint32_t* dataPacketCount, uint32_t* onePacketSize, uint32_t* flashOffset);
void flashDataByFile(HANDLE handle, DWORD* dwWritten, FILE* fp, uint8_t* checkSum);
