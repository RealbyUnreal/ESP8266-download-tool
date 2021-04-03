#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
void dataBufWrite(BYTE* data, uint32_t bufSize, uint32_t* inputData);
int8_t checkSumCalculate(FILE* fp);
void flashDataByFile(FILE* fp, uint8_t checkSum);

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
	int8_t x;

	FILE* fp = fopen("factory_WROOM-02N_portChangedByJ.Min.bin", "rb");

	if (fp == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}

	x = checkSumCalculate(fp);

	fseek(fp, 0, SEEK_SET);

	flashDataByFile(fp, x);

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
	uint8_t* temp = (uint8_t*)malloc(0x1FFF);
	uint32_t i = 0;
	beginWriteCmdType cmd;
	uint8_t startEndSignal = 0xC0;
	uint8_t* cp = &cmd;
	uint32_t inputData[4] = { 0x1000, 0x00, 0x00,0x00 };

	cmd.startSignal = startEndSignal;
	cmd.direction = 0x00;
	cmd.order = 0x03;
	cmd.size[0] = 0x00;
	cmd.size[1] = 0x10;
	cmd.checkSum[1] = 0x00;
	cmd.checkSum[2] = 0x00;
	cmd.checkSum[3] = 0x00;


	for (int j = 0; j < 0xc0; j++)
	{
		inputData[1] = j * 0x1000;
		dataBufWrite(cmd.dataBuffer, 16, inputData);
		cmd.checkSum[0] = checkSum;

		for (i = 0; i < 25; i++)
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

		for (int k = 0; k < 0x1000 + 26; k++)
		{
			printf("%02X\t", *(temp + k));
		}
	}

	free(temp);
}
