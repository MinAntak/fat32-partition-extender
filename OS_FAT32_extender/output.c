#include <stdio.h>
#include <Windows.h>
#include "fat32.h"
#include "input.h"
#include "output.h"

void saveToOutputFile(struct fat32_BPB newFat32BPB, struct fat32_BPB oldFat32BPB, HANDLE oldPartition, DWORD bytesRead, BYTE* oldSector) {
	BYTE sector[BUFFER];
	HANDLE device = NULL;
	DWORD dwBytesToWrite = BUFFER;
	DWORD dwBytesWritten = 0;

	device = CreateFile("output.img", GENERIC_WRITE, 0,
		NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	if (device == INVALID_HANDLE_VALUE)
	{
		printf("Blad przy otwieraniu");
		return;
	}
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
	unsigned int sectorsRead = 0;
	//RESERVED
	printf(">Tworzenie nowego obszaru zarezerwowanego \n");
	getNewFirstSector(newFat32BPB, oldSector);

	if (!WriteFile(device, oldSector, dwBytesToWrite, &dwBytesWritten, NULL)) {
		printf("Blad przy odczycie");
		return;
	}

	for (int i = 0; i < newFat32BPB.fatBPB.reserved_sector_count; i++)
	{
		for (int j = 0; j < (newFat32BPB.fatBPB.bytes_per_sector / BUFFER); j++)
		{
			if (i == 0 && j == 0) {
				j = 1;
				if (j >= (newFat32BPB.fatBPB.bytes_per_sector / BUFFER)) {
					break;
				}
			}

			if (!ReadFile(oldPartition, sector, BUFFER, &bytesRead, NULL))
			{
				printf("Blad przy odczycie");
				return;
			}

			if (i == newFat32BPB.fatEBPB.backup_BS_sector && j == 0) {
				getNewFirstSector(newFat32BPB, sector);
			}

			if (!WriteFile(device, sector, dwBytesToWrite, &dwBytesWritten, NULL)) {
				printf("Blad przy zapisie");
				return;
			}
		}
		sectorsRead++;
	}

	//FAT TABLE
	printf(">Rozszerzanie tablicy FAT \n");
	if (!ReadFile(oldPartition, sector, BUFFER, &bytesRead, NULL))
	{
		printf("Blad przy odczycie");
		return;
	}
	for (int k = 0; k < newFat32BPB.fatBPB.table_count; k++)
	{
		unsigned int fatBytesRead = 0;
		unsigned int oldClustersNumber = getClustersNumber(oldFat32BPB.fatBPB, getDataSectors(oldFat32BPB.fatBPB, oldFat32BPB.fatEBPB));
		unsigned int actualSectorPart = 0;
		BYTE fatTablePart[BUFFER];
		cleanFatTable(fatTablePart);
		for (unsigned int i = 0; i < getClustersNumber(newFat32BPB.fatBPB, newFat32BPB.newSizeSectors); i++)
		{
			if (i < oldClustersNumber) {
				for (int j = 0; j < 4; j++)
				{
					fatTablePart[fatBytesRead] = sector[fatBytesRead];
					fatBytesRead++;
				}

				if (fatBytesRead == BUFFER) {
					if (!WriteFile(device, sector, dwBytesToWrite, &dwBytesWritten, NULL)) {
						printf("Blad przy zapisie");
						return;
					}

					if (!ReadFile(oldPartition, sector, BUFFER, &bytesRead, NULL))
					{
						printf("Blad przy odczycie");
						return;
					}
					actualSectorPart++;
					if (actualSectorPart == newFat32BPB.fatBPB.bytes_per_sector / BUFFER) {
						sectorsRead++;
						actualSectorPart = 0;
					}

					cleanFatTable(fatTablePart);
					fatBytesRead = 0;
				}
			}
			else {
				fatBytesRead += 4;
				if (fatBytesRead == BUFFER) {
					if (!WriteFile(device, sector, dwBytesToWrite, &dwBytesWritten, NULL)) {
						printf("Blad przy zapisie");
						return;
					}

					cleanFatTable(fatTablePart);
					fatBytesRead = 0;
				}
			}
		}
		if (fatBytesRead != 0) {
			if (!WriteFile(device, sector, dwBytesToWrite, &dwBytesWritten, NULL)) {
				printf("Blad przy zapisie");
				return;
			}
		}

		while (sectorsRead < (((k + 1) * oldFat32BPB.fatEBPB.table_size_32) + oldFat32BPB.fatBPB.reserved_sector_count)) {
			if (!ReadFile(oldPartition, sector, BUFFER, &bytesRead, NULL))
			{
				printf("Blad przy odczycie");
				return;
			}
			actualSectorPart++;
			if (actualSectorPart == newFat32BPB.fatBPB.bytes_per_sector / BUFFER) {
				sectorsRead++;
				actualSectorPart = 0;
			}
		}
	}

	//Data region
	printf(">Rozszerzanie obszaru danych \n");
	BYTE fatTablePart[512];
	cleanFatTable(fatTablePart);
	for (unsigned int i = 2; i < getClustersNumber(newFat32BPB.fatBPB, newFat32BPB.newSizeSectors); i++)
	{
		unsigned int fatBytesRead = 0;
		unsigned int oldClustersNumber = getClustersNumber(oldFat32BPB.fatBPB, getDataSectors(oldFat32BPB.fatBPB, oldFat32BPB.fatEBPB));
		unsigned int actualSectorPart = 0;
		if (i < oldClustersNumber) {
			for (int j = 0; j < (newFat32BPB.fatBPB.bytes_per_sector * newFat32BPB.fatBPB.sectors_per_cluster) / BUFFER; j++)
			{
				if (!WriteFile(device, sector, dwBytesToWrite, &dwBytesWritten, NULL)) {
					printf("Blad przy zapisie");
					return;
				}
				if (!ReadFile(oldPartition, sector, BUFFER, &bytesRead, NULL))
				{
					printf("Blad przy odczycie");
					return;
				}
			}
		}
		else {
			for (int j = 0; j < (newFat32BPB.fatBPB.bytes_per_sector * newFat32BPB.fatBPB.sectors_per_cluster) / BUFFER; j++)
			{
				if (!WriteFile(device, sector, dwBytesToWrite, &dwBytesWritten, NULL)) {
					printf("Blad przy zapisie");
					return;
				}
			}
		}
	}

	CloseHandle(device);
	printf(">Rozszerzanie obrazu partycji zakonczone pomyslnie \n");
}

void getNewFirstSector(struct fat32_BPB newFat32BPB, BYTE* oldSector) {
	BYTE* tempValue;

	oldSector[19] = 0; //TotalSectors_16
	oldSector[20] = 0;

	oldSector[22] = 0; //FatSize_16
	oldSector[23] = 0;

	tempValue = changeIntToByteArray(newFat32BPB.fatBPB.total_sectors_32);
	for (int i = 0; i < 4; i++)
	{
		oldSector[32 + i] = tempValue[i]; //TotalSectors_32
	}

	tempValue = changeIntToByteArray(newFat32BPB.fatEBPB.table_size_32);
	for (int i = 0; i < 4; i++)
	{
		oldSector[36 + i] = tempValue[i]; //FatSize_32
	}
}


BYTE* changeIntToByteArray(unsigned int value) {
	BYTE returnByte[4];

	returnByte[3] = (value >> 24) & 0xFF;
	returnByte[2] = (value >> 16) & 0xFF;
	returnByte[1] = (value >> 8) & 0xFF;
	returnByte[0] = value & 0xFF;

	return returnByte;
}

void cleanFatTable(BYTE* table) {
	for (int i = 0; i < BUFFER; i++)
	{
		table[i] = 0;
	}
}
