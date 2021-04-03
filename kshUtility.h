#pragma once
#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

void dataBufWrite(uint8_t* dataBuf, uint32_t bufSize, uint32_t* inputData);
int8_t checkSumCalculate(FILE* fp, int8_t* data);

bool writeData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten);
bool readData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwRead, UINT timeout);