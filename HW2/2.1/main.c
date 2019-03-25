//******************************************
//Jonathan Feige
//CS4900
//P2
//Takes a binary file and gives Super Block
//data and group descriptor data.
//******************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ext2fs.h"

#define TMUFS_FN_LENGTH 10
#define TMUFS_ATTRIBUTE_ASCII    0x01
#define TMUFS_ATTRIBUTE_BINARY   0x02
#define TMUFS_ATTRIBUTE_HIDDEN   0x04
#define TMUFS_ATTRIBUTE_DELETED  0x08
#define MAX( a, b ) ( ( a > b) ? a : b )
#define MIN( a, b ) ( ( a < b) ? a : b )
	int main()
	{	
		char file_name[255];
		FILE *ptr;
		
		printf("Enter name of a file you wish to see\n");
		scanf("%s",file_name);
	
		ptr = fopen(file_name,"rb");  // r for read, b for binary

		if (ptr == NULL)
		{
			perror("Error while opening the file.\n");
			exit(EXIT_FAILURE);
		}
   
		unsigned char buffer[1024];
		struct ext2_super_block super;
		
		fread(buffer,sizeof(buffer),1,ptr);
		fread(buffer,sizeof(buffer),1,ptr);
			
		printf("\n");
		
		super.s_magic = (buffer[57]<<8) | buffer[56];
		if(super.s_magic != 61267)
		{
			perror("Error not EXT2\n");
			exit(EXIT_FAILURE);
		}
		
		//EXT2 Super Block
		super.s_inodes_count = (buffer[3]<<24) | (buffer[2]<<16)| (buffer[1]<<8) | buffer[0];
		super.s_blocks_count = (buffer[7]<<24) | (buffer[6]<<16)| (buffer[5]<<8) | buffer[4];
		super.s_r_blocks_count = (buffer[11]<<24) | (buffer[10]<<16)| (buffer[9]<<8) | buffer[8];
		super.s_free_blocks_count = (buffer[15]<<24) | (buffer[14]<<16)| (buffer[13]<<8) | buffer[12];
		super.s_free_inodes_count = (buffer[19]<<24) | (buffer[18]<<16)| (buffer[17]<<8) | buffer[16];
		super.s_first_data_block = (buffer[23]<<24) | (buffer[22]<<16)| (buffer[21]<<8) | buffer[20];
		super.s_log_block_size = (buffer[27]<<24) | (buffer[26]<<16)| (buffer[25]<<8) | buffer[24];
		super.s_log_frag_size = (buffer[31]<<24) | (buffer[30]<<16)| (buffer[29]<<8) | buffer[28];
		super.s_blocks_per_group = (buffer[35]<<24) | (buffer[34]<<16)| (buffer[33]<<8) | buffer[32];
		super.s_frags_per_group = (buffer[39]<<24) | (buffer[38]<<16)| (buffer[37]<<8) | buffer[36];
		super.s_inodes_per_group = (buffer[43]<<24) | (buffer[42]<<16)| (buffer[41]<<8) | buffer[40];
		super.s_mtime = (buffer[47]<<24) | (buffer[46]<<16)| (buffer[45]<<8) | buffer[44];
		super.s_wtime = (buffer[51]<<24) | (buffer[50]<<16)| (buffer[49]<<8) | buffer[48];
		super.s_mnt_count = (buffer[53]<<8) | buffer[52];
		super.s_max_mnt_count = (buffer[55]<<8) | buffer[54];
		super.s_state = (buffer[59]<<8) | buffer[58];
		super.s_errors = (buffer[61]<<8) | buffer[60];
		super.s_minor_rev_level = (buffer[63]<<8) | buffer[62];
		super.s_lastcheck = (buffer[67]<<24) | (buffer[66]<<16)| (buffer[65]<<8) | buffer[64];
		super.s_checkinterval = (buffer[71]<<24) | (buffer[70]<<16)| (buffer[69]<<8) | buffer[68];
		super.s_creator_os = (buffer[75]<<24) | (buffer[74]<<16)| (buffer[73]<<8) | buffer[72];
		super.s_rev_level = (buffer[79]<<24) | (buffer[78]<<16)| (buffer[77]<<8) | buffer[76];
		super.s_def_resuid = (buffer[83]<<24) | (buffer[82]<<16)| (buffer[81]<<8) | buffer[80];
		super.s_def_resgid = (buffer[87]<<24) | (buffer[86]<<16)| (buffer[85]<<8) | buffer[84];
		
		//EXT2_DYNAMIC_REV Only
		super.s_first_ino = (buffer[91]<<24) | (buffer[90]<<16)| (buffer[89]<<8) | buffer[88];
		super.s_inode_size = (buffer[93]<<8) | buffer[92];
		super.s_block_group_nr = (buffer[95]<<8) | buffer[94];
		super.s_feature_compat = (buffer[99]<<24) | (buffer[98]<<16)| (buffer[97]<<8) | buffer[96];
		super.s_feature_incompat = (buffer[103]<<24) | (buffer[102]<<16)| (buffer[101]<<8) | buffer[100];
		super.s_feature_ro_compat = (buffer[107]<<24) | (buffer[106]<<16)| (buffer[105]<<8) | buffer[104];
		
		for(int i = 0; i < 16; i++)
		{
			super.s_uuid[i] = buffer[123 - i];
		} // Ends at buffer[123]
		
		for(int i = 0; i < 16; i++)
		{
			super.s_volume_name[i] = buffer[139 - i]; //char
		} // Ends at buffer[139]
		
		for(int i = 0; i < 64; i++)
		{
			super.s_uuid[i] = buffer[203 - i]; ////char
		} // Ends at buffer[203]
		
		super.s_algorithm_usage_bitmap = (buffer[207]<<24) | (buffer[206]<<16)| (buffer[205]<<8) | buffer[204];
		
		//EXT2_COMPAT_PREALLOC on
		
		super.s_prealloc_blocks = buffer[208];
		super.s_prealloc_dir_blocks = buffer[209];
		super.s_padding1 = (buffer[211]<<8) | buffer[210];
		
		int inode_size = MAX( super.s_inode_size, sizeof(struct ext2_inode));
		int block_size = 1 << (10 + super.s_log_block_size);
		int frag_size = 1 << (10 + super.s_log_frag_size);
		int num_block = floor(super.s_blocks_count/super.s_blocks_per_group);
		
		
		printf("Superblock magic Number: %x\n", super.s_magic);
		printf("Inodes Count: %d\n", super.s_inodes_count);
		printf("Blocks Count: %d\n", super.s_blocks_count);
		printf("Free Blocks Count: %d\n", super.s_free_blocks_count);
		printf("Free Inodes Count: %d\n", super.s_free_inodes_count);
		printf("First Data Block: %d\n", super.s_first_data_block);
		printf("Block Size: %d\n", block_size);
		printf("Framgent Size: %d\n", frag_size);
		printf("Blocks per Group: %d\n", super.s_blocks_per_group);
		printf("Fragments per Group: %d\n", super.s_frags_per_group);
		printf("Inodes per Group: %d\n", super.s_inodes_per_group);
		printf("Number of Block Groups: %d\n", num_block +1);
		printf("Number of Pre-Allocate Blocks: %d\n",super.s_def_resuid);
		printf("Number of Pre-Allocate Directory Blocks: %d\n",super.s_def_resgid);
		printf("Inode size: %d\n", inode_size);
		
		int group_block = (super.s_first_data_block + 1)*block_size;
		
		unsigned char block_buffer[block_size];
		struct ext2_group_desc groupDec;
		struct ext2_group_desc* gdptr;

		fseek(ptr, group_block, SEEK_SET );	
		fread(block_buffer, block_size, 1, ptr);
		
		for (int i = 0; i <= num_block; i++)
		{	
			gdptr = (struct ext2_group_desc*) &block_buffer[i * sizeof(groupDec)];
			
			int top = MIN(super.s_blocks_count - 1, (super.s_blocks_per_group * (i + 1) - 1));
			printf("Information for Block Group %d\n", i);
			printf("----------------------------------------------------------------------------\n");
			printf("Group range: %d to %d\n", (super.s_blocks_per_group * i), top);
			printf("Blocks Bitmap block at: %d\n", gdptr -> bg_block_bitmap);
			printf("Inodes Bitmap block at: %d\n", gdptr -> bg_inode_bitmap);
			printf("Inodes Table block at: %d\n", gdptr -> bg_inode_table);
			printf("Free Blocks Count: %d\n", gdptr -> bg_free_blocks_count);
			printf("Free Inodes Count: %d\n", gdptr -> bg_free_inodes_count);
			printf("Used Directories Count: %d\n", gdptr -> bg_used_dirs_count);
			
			printf("----------------------------------------------------------------------------\n\n");	
		}
	}