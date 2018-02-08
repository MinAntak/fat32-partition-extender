#define MIN_CLUSTERS 65525
#define MAX_CLUSTERS  268435445
#define MB (1024 * 1024)

struct fat_BPB
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;
};

struct fat_extBPB_32
{
	unsigned int		table_size_32;
	unsigned short		extended_flags;
	unsigned short		fat_version;
	unsigned int		root_cluster;
	unsigned short		fat_info;
	unsigned short		backup_BS_sector;
	unsigned char 		reserved_0[12];
	unsigned char		drive_number;
	unsigned char 		reserved_1;
	unsigned char		boot_signature;
	unsigned int 		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

};

struct fat32_BPB {
	struct fat_BPB fatBPB;
	struct fat_extBPB_32 fatEBPB;
	unsigned int newSizeSectors;
};

struct fat_BPB getFatBPB(BYTE* sector);
struct fat_extBPB_32 getFatEBPB(BYTE* sector);
unsigned short convertToUnsignedShort(unsigned char* bytes);
unsigned int convertToUnsignedInt(unsigned char* bytes);
unsigned short tempByteShort(BYTE* sector, int position);
unsigned int tempByteInt(BYTE* sector, int position);
void printFAT32PartitionData(struct fat_BPB fatBPB,struct fat_extBPB_32 fatEBPB);
int checkFATSystemDrive(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB);
unsigned int getFatTableSize(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB);
unsigned int getDataSectors(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB);
unsigned int getTotalSectors(struct fat_BPB fatBPB);
unsigned int getNewSize(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB);
struct fat32_BPB getNewBPB(struct fat_BPB fatBPB, struct fat_extBPB_32 fatEBPB);
unsigned int changeSectorNumber(struct fat_BPB fatBPB, unsigned int newSize);
unsigned int changeFATSize(struct fat_BPB fatBPB, unsigned int newSize);
unsigned int getClustersNumber(struct fat_BPB fatBPB, unsigned int dataSectors);