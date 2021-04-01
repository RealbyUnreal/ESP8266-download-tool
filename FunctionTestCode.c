#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
void dataBufWrite(BYTE* data, uint32_t bufSize, uint32_t* inputData);
int8_t checkSumCalculate(FILE* fp);
void flashDataByFile(FILE* fp, uint32_t fileSize, uint32_t checkSum);

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
#pragma pack(pop)

int main()
{
	/*
	commandRequestType a;

	uint32_t dataSize = sizeof(a);
	uint8_t databuffer[3] = { 0 };
	uint32_t inputData[6] = { 0x12345678, 0x87654321, 0x10203040, 0x40302010, 0x50607080, 0x80706050};

	dataBufWrite(&a, dataSize, inputData);

	BYTE* p = &a;
	*/
	char fileName[50] = "factory_WROOM-02N_portChangedByJ.Min.bin";
	int8_t x;
	uint32_t y = 1;

	FILE* fp = fopen(fileName, "rb");

	if (fp == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}
	
	x = checkSumCalculate(fp);
	y = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	flashDataByFile(fp, y, x);

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
	
	while (feof(fp) == 0)
	{
		fread(&temp, sizeof(uint8_t), 1, fp);

		checkSum ^= temp;
	}
	
	return checkSum;
}

void flashDataByFile(FILE* fp, uint32_t fileSize, uint32_t checkSum)
{
	uint8_t* temp = (uint8_t*)malloc(fileSize + 27);
	uint32_t i = 0;
	beginWriteCmdType cmd;
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t inputData[4] = { fileSize, 0x00, 0x00, 0x00 };

	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = 0x03;
	cmd.size[0] = 0x00;
	cmd.size[1] = 0x00;
	dataBufWrite(cmd.checkSum, 4, &checkSum);
	dataBufWrite(cmd.dataBuffer, 16, inputData);

	for (i = 0; i < 25; i++)
	{
		*(temp+i) = cp[i];
	}

	while (feof(fp) == 0)
	{
		fread((temp + i), sizeof(uint8_t), 1, fp);
		i++;
	}

	*(temp + i) = startEndSignal;

	for (uint32_t j = 0; j <  (fileSize + 26); j++)
	{
		printf("%x \t", *(temp+j));
	}

	printf("%x", *(temp + i));
	free(temp);
}