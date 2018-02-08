#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include "input.h"
#include "fat32.h"
#include "output.h"


char* readFilename() {
	char fileName[255];
	printf("Podaj nazwe pliku:");
	scanf("%s", &fileName);

	int lenght;
	for (int i = 0; i < 255; i++)
	{
		if (fileName[i] == 0) {
			lenght = i;
			break;
		}
	}

	char* returnFileName = (char*)malloc(lenght);

	for (int i = 0; i < lenght + 1; i++)
	{
		returnFileName[i] = fileName[i];
	}

	return returnFileName;
}

void openFile() {
	BYTE sector[512];
	DWORD bytesRead;
	HANDLE device = NULL;

	char* fileName = readFilename();

	device = CreateFile(fileName, GENERIC_READ,
		NULL, NULL, OPEN_EXISTING, 0, NULL);

	if (device == INVALID_HANDLE_VALUE)
	{
		printf("Blad przy otwieraniu");
		return;
	}

	SetFilePointer(device, 0, NULL, FILE_BEGIN);

	if (!ReadFile(device, sector, 512, &bytesRead, NULL))
	{
		printf("Blad przy odczycie");
		return;
	}

	struct fat32_BPB oldFatBPB;
	oldFatBPB.fatBPB = getFatBPB(sector);
	oldFatBPB.fatEBPB = getFatEBPB(sector);
	printFAT32PartitionData(oldFatBPB.fatBPB, oldFatBPB.fatEBPB);
	if (checkFATSystemDrive(oldFatBPB.fatBPB, oldFatBPB.fatEBPB)) {
		struct fat32_BPB newFat32BPB = getNewBPB(oldFatBPB.fatBPB, oldFatBPB.fatEBPB);
		saveToOutputFile(newFat32BPB, oldFatBPB, device, bytesRead, sector);
		CloseHandle(device);
	}
	else {
		CloseHandle(device);
		return;
	}

}
