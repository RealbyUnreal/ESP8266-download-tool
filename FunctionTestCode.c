#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
	ESP_FLASH_BEGIN = 0x02,
	ESP_FLASH_DATA = 0x03,
	//ESP_FLASH_END = 0x04,
	ESP_MEM_BEGIN = 0x05,
	ESP_MEM_END = 0x06,
	ESP_MEM_DATA = 0x07,
	//ESP_SYNC = 0x08,
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
	int8_t checkSum[4];
	uint8_t dataBuffer[16];
	uint8_t endSignal;
}beginWriteCmdType; 

typedef struct NODE
{
	struct NODE* next;
	BYTE data;
}node;
#pragma pack(pop)

void dataBufWrite(BYTE* data, uint32_t bufSize, uint32_t* inputData);
int8_t checkSumCalculate(FILE* fp);
void flashDataByFile(FILE* fp, uint8_t checkSum);
void addNode(node* target, BYTE data);
void freeNodeAll(node* head);
void removeNextNode(node* target);
void readNodeAll(node* curr);
void memBegin();
void memFlashBeginMsg(uint32_t* eraseSize, uint32_t* dataPacketCount, uint32_t* onePacketSize, uint32_t* flashOffset, int type);

int main()
{
	int8_t x;
	node* head = (node*)malloc(sizeof(node));
	FILE* fp = fopen("factory_WROOM-02N_portChangedByJ.Min.bin", "rb");

	if (fp == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}
	
	x = checkSumCalculate(fp);
	fseek(fp, 0, SEEK_SET);
	memBegin();

	fclose(fp);
	return 0;
}

void dataBufWrite(BYTE* data, uint32_t bufSize, uint32_t* inputData)
{
	int k = 0;

	for (uint32_t i = 0; i < bufSize; i++)
	{
		if (i != 0 && (i % 4) == 0)
			k++;

		data[i] = (*(inputData + k) >> (8 * i)) & 0xFF;			
	}
}

int8_t checkSumCalculate(FILE* fp)
{
	int8_t temp = 0, checkSum = 0xEF;
	
	for (int i = 0; i < 0x1000; i++)
	{
		fread(&temp, sizeof(uint8_t), 1, fp);

		checkSum ^= temp;
	}

	return checkSum;
}

void flashDataByFile(FILE* fp, uint8_t checkSum)
{
	//node* head = (node*)malloc(sizeof(node));
	//head->next = NULL;
	//head->data = 0;

	//for (uint32_t i = 0; i < count; i++)
	//{
	//	addNode(head, 0);
	//}

	uint32_t count = 0x1000 + 26;
	uint8_t temp[0x1FFF] = { 0 };
	beginWriteCmdType cmd;
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t tempIndex, countGap = 0;
	uint32_t inputData[4] = { 0x1000, 0x00, 0x00,0x00 }; //"Data to write", length,Sequence number, 0, 0

	//
	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = ESP_FLASH_DATA;
	cmd.size[0] = 0x00;
	cmd.size[1] = 0x10;
	cmd.checkSum[1] = 0x00;
	cmd.checkSum[2] = 0x00;
	cmd.checkSum[3] = 0x00;

	for (int j = 0; j < 1; j++)
	{
		tempIndex = 1;
		cmd.checkSum[0] = checkSum;
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

		for (int i = 0; i < tempIndex; i++)
		{
			printf("%02X\t", temp[i]);
		}
	}
	//
}

void addNode(node* target, BYTE data)
{
	node* newNode = (node*)malloc(sizeof(node));

	newNode->next = target->next;
	newNode->data = data;

	target->next = newNode;
}

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

void readNodeAll(node* curr)
{
	while (curr != NULL)
	{
		printf("%02x\t", curr->data);
		curr = curr->next;
	}
}

void memBegin()
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

	temp[tempIndex] = startEndSignal;
	tempIndex++;

	for (int i = 0; i < tempIndex; i++)
	{
		printf("%02X\t", temp[i]);
	}
}

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