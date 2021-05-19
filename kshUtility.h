#pragma once
#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef struct NODE
{
	struct NODE* next;
	BYTE data;
}node;
#pragma pack(pop)

void dataBufWrite(uint8_t* dataBuf, uint32_t bufSize, uint32_t* inputData);
void checkSumCalculate(FILE* fp, int8_t* data);

bool writeData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten);
bool readData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwRead, UINT timeout);
void inputComPort(wchar_t* comPortMask);
void inputFileName(char* fileName);

void addNode(node* target, BYTE data);
void freeNodeAll(node* head);
void removeNextNode(node* target);
int readPacket(HANDLE hRead);