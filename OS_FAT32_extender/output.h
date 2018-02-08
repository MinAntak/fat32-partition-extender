#define BUFFER 512

void saveToOutputFile(struct fat32_BPB newFat32BPB, struct fat32_BPB oldFat32BPB, HANDLE oldPartition, DWORD bytesRead, BYTE* oldSector);
void getNewFirstSector(struct fat32_BPB newFat32BPB, BYTE* oldSector);
BYTE* changeIntToByteArray(unsigned int value);
void cleanFatTable(BYTE* table);