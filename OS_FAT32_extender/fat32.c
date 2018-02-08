#include <stdio.h>
#include <Windows.h>
#include "fat32.h"

struct fat_BPB getFatBPB(BYTE* sector) {
	struct fat_BPB fatBPB;
	int sectorPosition = 0;
	for (int i = 0; i < 3; i++)
	{
		fatBPB.bootjmp[i] = sector[i];
	}
	sectorPosition += 3;

	for (int i = 0; i < 8; i++)
	{
		fatBPB.oem_name[i] = sector[sectorPosition + i];
	}
	sectorPosition += 8;

	fatBPB.bytes_per_sector = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	fatBPB.sectors_per_cluster = sector[sectorPosition];
	sectorPosition++;

	fatBPB.reserved_sector_count = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	fatBPB.table_count = sector[sectorPosition];
	sectorPosition++;

	fatBPB.root_entry_count = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	fatBPB.total_sectors_16 = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	fatBPB.media_type = sector[sectorPosition];
	sectorPosition++;

	fatBPB.table_size_16 = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	fatBPB.sectors_per_track = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	fatBPB.head_side_count = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	fatBPB.hidden_sector_count = tempByteInt(sector, sectorPosition);
	sectorPosition += 4;

	fatBPB.total_sectors_32 = tempByteInt(sector, sectorPosition);
	sectorPosition += 4;

	return fatBPB;
}

struct fat_extBPB_32 getFatEBPB(BYTE* sector) {
	struct fat_extBPB_32 fatEBPB;
	int sectorPosition = 36;

	fatEBPB.table_size_32 = tempByteInt(sector, sectorPosition);
	sectorPosition += 4;

	/*
	extended flags +2
	fat_version +2
	*/
	sectorPosition += 4;

	fatEBPB.root_cluster = tempByteInt(sector, sectorPosition);
	sectorPosition += 4;

	fatEBPB.fat_info = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	fatEBPB.backup_BS_sector = tempByteShort(sector, sectorPosition);
	sectorPosition += 2;

	return fatEBPB;

}


unsigned short convertToUnsignedShort(unsigned char* bytes) {
	unsigned short returnValue = (bytes[1] << 8) | bytes[0];
	return returnValue;
}


unsigned int convertToUnsignedInt(unsigned char* bytes) {
	unsigned int returnValue = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
	return returnValue;
}

unsigned short tempByteShort(BYTE* sector, int position) {
	unsigned char tempShort[2];
	for (int i = 0; i < 2; i++)
	{
		tempShort[i] = sector[position + i];
	}

	return convertToUnsignedShort(tempShort);
}

unsigned int tempByteInt(BYTE* sector, int position) {
	unsigned char tempInt[4];
	for (int i = 0; i < 4; i++)
	{
		tempInt[i] = sector[position + i];
	}

	return convertToUnsignedInt(tempInt);
}

void printFAT32PartitionData(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB) {
	printf("OEM_name %s \n", fatBPB.oem_name);
	printf("Bajtow na sektor %d \n", fatBPB.bytes_per_sector);
	printf("Sektorow na klaster %d \n", fatBPB.sectors_per_cluster);
	printf("Zarezerwowanych sektorow %d \n", fatBPB.reserved_sector_count);
	printf("Liczba tablic FAT %d \n", fatBPB.table_count);
	printf("Katalog glowny %d \n", fatBPB.root_entry_count);
	printf("Sektory (16bit) %d \n", fatBPB.total_sectors_16);
	printf("Typ dysku %d \n", fatBPB.media_type);
	printf("Rozmiar tablicy FAT (16bit) %d \n", fatBPB.table_size_16);
	printf("Sektory na sciezke %d \n", fatBPB.sectors_per_track);
	printf("Liczba glowic %d \n", fatBPB.head_side_count);
	printf("Ukryte sektory %d \n", fatBPB.hidden_sector_count);
	printf("Sektory (32bit) %d \n", fatBPB.total_sectors_32);

	printf("Rozmiar tablicy FAT (32bit) %d \n", fatEBPB.table_size_32);
	printf("Klaster katalogu domowego %d \n", fatEBPB.root_cluster);
	printf("Kopia zapasowa BPB i EBP %d \n", fatEBPB.backup_BS_sector);
	printf("------------------------------------------------------- \n");

}

int checkFATSystemDrive(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB) {
	unsigned int clusterNumber = getDataSectors(fatBPB, fatEBPB) / fatBPB.sectors_per_cluster;
	if (clusterNumber < MIN_CLUSTERS || clusterNumber > MAX_CLUSTERS) //FAT12 or FAT16 or exFAT
	{
		printf("Partycja nie jest w systemie plików FAT32 \n");
		return 0;
	}
	return 1;
}

unsigned int getTotalSectors(struct fat_BPB fatBPB) {
	unsigned int totalSectors;

	if (fatBPB.total_sectors_16 != 0) {
		totalSectors = fatBPB.total_sectors_16;
	}
	else if (fatBPB.total_sectors_32 != 0) {
		totalSectors = fatBPB.total_sectors_32;
	}
	else {
		printf("Niepoprawna partycja \n");
		return 0;
	}

	return totalSectors;
}

unsigned int getFatTableSize(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB) {
	unsigned int fatSize;
	if (fatBPB.table_size_16 != 0) {
		fatSize = fatBPB.table_size_16;
	}
	else if (fatEBPB.table_size_32 != 0) {
		fatSize = fatEBPB.table_size_32;
	}
	else {
		printf("Niepoprawna partycja \n");
		return 0;
	}

	return fatSize;
}

unsigned int getDataSectors(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB) {
	return getTotalSectors(fatBPB) - (fatBPB.reserved_sector_count) - (fatBPB.table_count * getFatTableSize(fatBPB, fatEBPB));
}

unsigned int getNewSize(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB) {
	unsigned int totalSectors = getTotalSectors(fatBPB);
	unsigned int partitionActualSizeMB = (totalSectors * fatBPB.bytes_per_sector) / MB;

	printf("Aktualny rozmiar partycji: %dMB \n", partitionActualSizeMB);

	unsigned int dataSectors = getDataSectors(fatBPB, fatEBPB);
	unsigned int partitionDataSizeMB = (dataSectors * fatBPB.bytes_per_sector);
	partitionDataSizeMB /= MB;
	printf("Aktualny rozmiar obszaru danych: %dMB \n", partitionDataSizeMB);

	unsigned long long int maxPartitionNewSizeMB = fatBPB.bytes_per_sector * fatBPB.sectors_per_cluster;
	maxPartitionNewSizeMB *= MAX_CLUSTERS;
	maxPartitionNewSizeMB /= MB;
	printf("Maksymalny nowy dozwolony rozmiar obszaru danych: %dMB \n", maxPartitionNewSizeMB);

	printf("Podaj nowy rozmiar obszaru danych (w MB): ");
	unsigned int newSize = 0;
	scanf_s("%d", &newSize);

	if (newSize > partitionActualSizeMB && newSize < maxPartitionNewSizeMB) {
		return newSize;
	}
	
	printf("Niepoprawny nowy rozmiar partycji. Podaj ponownie. \n");
	return getNewSize(fatBPB, fatEBPB);
}

struct fat32_BPB getNewBPB(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB) {
	unsigned int newSizeMB = getNewSize(fatBPB, fatEBPB);

	unsigned long long int newSize = newSizeMB;
	newSize *= MB;
	newSize /= fatBPB.bytes_per_sector;

	fatBPB.total_sectors_32 = changeSectorNumber(fatBPB, newSize);
	fatEBPB.table_size_32 = changeFATSize(fatBPB, newSize);
	fatBPB.total_sectors_16 = 0;
	fatBPB.table_size_16 = 0;

	struct fat32_BPB fat32BPB;
	fat32BPB.fatBPB = fatBPB;
	fat32BPB.fatEBPB = fatEBPB;
	fat32BPB.newSizeSectors = newSize;

	return fat32BPB;
}

unsigned int changeSectorNumber(struct fat_BPB fatBPB, unsigned int newSize) {
	return fatBPB.reserved_sector_count + (fatBPB.table_count * changeFATSize(fatBPB, newSize)) + newSize;
}

unsigned int changeFATSize(struct fat_BPB fatBPB, unsigned int newSize) {
	unsigned int newClustersNumber = getClustersNumber(fatBPB, newSize);

	return (newClustersNumber * 4) / fatBPB.bytes_per_sector;
}

unsigned int getClustersNumber(struct fat_BPB fatBPB, unsigned int dataSectors) {
	return dataSectors / fatBPB.sectors_per_cluster;
}

