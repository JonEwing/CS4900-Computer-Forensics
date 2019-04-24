////////////////////////////////////////////////////////////////////////////////////////
// Author: Jonathan Feige
// File: extractor.cc
// Date: 04/19/2019
// Description: Takes an EXT2 file system and searches for all deleated data in
// the system from the three possible methods. Can be run with make then make run, 
//or:				
//						g++ -g -o ext.o extractor.cc
//						./ext.o *file_system*
////////////////////////////////////////////////////////////////////////////////////////


#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pwd.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include<stdio.h> 
#include "ext2fs.h"

#define SUPERBLOCK_OFFSET 1024
#define MAX_BLOCK_SIZE 4096
#define MAX_NUMBER_BLOCK_GROUPS 2000000
#define DIRECT_DATA_BLOCKS 12
#define SINGLE_INDIRECT_BLOCK 12
#define DOUBLE_INDIRECT_BLOCK 13

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"

#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KGR2  "\x1B[92m"
#define KYE2  "\x1B[93m"
#define KBL2  "\x1B[94m"
#define KMA2  "\x1B[94m"
#define KCY2  "\x1B[95m"
#define KWH2  "\x1B[96m"


using namespace std;

#define NO_COLORS 14
//                                   0     1    2     3     4     5     6     7     8     9     10    11     12    13    14   
const char* colors[NO_COLORS+1] = {KNRM, KGRN, KYEL, KBLU, KMAG, KCYN, KWHT, KYEL, KGR2, KYE2, KBL2, KMA2, KCY2, KWH2, KRED};
std::vector<int> dir;
std::vector<int> files;

__u32 total_blocks;
__u32 blocks_per_group;
__u32 total_block_groups;
__u32 block_size;
__u32 inode_size;
__u32 inodes_per_group;
__u32 inode_blocks_per_group;
__u32 group_descriptor_blocks;
__u32 reserved_blocks;
    
struct ext2_group_desc ext2_gd[MAX_NUMBER_BLOCK_GROUPS];

const unsigned int rights[9]={S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
const char* crights[3] = {"r", "w", "x"};

FILE* fptr;

int min(int a, int b)
{
	if( a <= b)
		return a;
	else
		return b;
}

int max(int a, int b)
{
	if( a >= b)
		return a;
	else
		return b;
}

struct ext2_inode get_inode(unsigned int inumber)
{
	unsigned int i_group = ( inumber - 1 ) / inodes_per_group;
    unsigned int i_index = ( inumber - 1 ) % inodes_per_group;

    unsigned int i_offset = block_size * ext2_gd[i_group].bg_inode_table + (i_index * inode_size);

    struct ext2_inode ext2_in;

    fseek(fptr, i_offset, SEEK_SET);
    fread((char*)&ext2_in, sizeof(struct ext2_inode), 1, fptr);
      
    return ext2_in;
}

int is_directory(unsigned int inumber)
{
	struct ext2_inode in;
	in = get_inode(inumber);
  
	if( ( in.i_mode & S_IFDIR ) == S_IFDIR )
		return 1;
	else
		return 0;
}

int print_directory_2(unsigned int inumber, char* ptr, int length, char* search)
{
	int i,j;
	std::vector<int> hold;
	unsigned int doffset;
	struct ext2_inode in;
	in = get_inode(inumber);
  
	// test for directory
	if( ( in.i_mode & S_IFDIR ) != S_IFDIR )
		return -1;
    
  
		struct ext2_dir_entry_2* dptr;
		dptr = (struct ext2_dir_entry_2*) ptr;
		doffset = 0;

		while(1)
		{
			if( doffset >= length)
			{
				break;
			}
      
			if( dptr->inode )
			{  
				char name[255];
				char date[255];
				struct ext2_inode in_t;
				struct tm* atime_tm;
				struct tm* ctime_tm;
				struct tm* mtime_tm;
				time_t a_rawtime;
				time_t c_rawtime;
				time_t m_rawtime;
        
				in_t = get_inode(dptr->inode);
				a_rawtime = in_t.i_atime;
				c_rawtime = in_t.i_ctime;
				m_rawtime = in_t.i_mtime;

				atime_tm = localtime(&a_rawtime);
				ctime_tm = localtime(&c_rawtime);
				mtime_tm = localtime(&m_rawtime);

        
				memcpy(name, dptr->name, dptr->name_len);
				name[dptr->name_len] = 0;

				if( search )
				{
					if(!strcmp(name, search))
					{
					return dptr->inode;
					} 
				}
				else
				{
					if( in_t.i_mode & S_IFDIR )
					{
						printf("%sd", KBL2);
						hold.push_back(dptr->inode);
					}
					else
					{
						printf("-");
						files.push_back(dptr->inode);
					}
            
					for(i = 0; i < 9; i++)
					{
						if( in_t.i_mode & rights[i] )
						{
							printf("%s", crights[i % 3]);
						}
						else
						{
							printf("-");
						}
					}
					for(i = 0; i < 9; i++)
					{
						if( in_t.i_mode & rights[i] )
						{
							printf("%s", crights[i % 3]);
						}
						else
						{
							printf("-");
						}
					}
					
					printf("\t");
					struct passwd pwd, *pwd_p;
					pwd_p = getpwuid(in_t.i_uid);
          
					if( pwd_p )
						printf("%s\t", pwd_p->pw_name);
					
					else
						printf("%d\t", in_t.i_uid);
					
					printf("%10u\t", in_t.i_size);        
					strftime(date, 255, "%b %d %R", mtime_tm);
					printf("%s\t", date);
					printf("%s%s\n", name, KNRM);

				}            
			}
			doffset += dptr->rec_len;    
			dptr = (struct ext2_dir_entry_2*) (ptr + doffset);
		}
  
	for (int i = 0; i< dir.size(); i++)
	{
		hold.push_back(dir[i]);
	}
	dir = hold;
    return 0;

}    
  
__u32 print_data_2(int inumber, char** ptr)
{
    int i,j,k;
    int nblocks;
    char* full_buf;
	std::vector<char> hold;

    unsigned int bytecount = 0;
    unsigned int bytepos = 0;
    
    struct ext2_inode ext2_in = get_inode(inumber);

    bytecount = ext2_in.i_size;
    
    full_buf = (char*) malloc( bytecount * sizeof(char));
        
    nblocks = ( ( ext2_in.i_blocks * 512 ) / block_size);

    for(i = 0; i < DIRECT_DATA_BLOCKS; i++)
    {
		char temp[MAX_BLOCK_SIZE];
	
		if(ext2_in.i_block[i] != 0)
		{
			fseek(fptr, (ext2_in.i_block[i] * block_size ), SEEK_SET);
			fread((char*)&temp, block_size, 1, fptr);
		}
		else
			memset(temp, 0, block_size);

		for(j = 0; j < min(block_size, bytecount); j++)
		{
			full_buf[bytepos++] = temp[j];
			hold.push_back(temp[j]);
			//printf("%c", temp[j]);
		}
		bytecount -= min(block_size, bytecount);

		if( !bytecount )
			break;
    }      

    //Single Indirect Block
    if( bytecount )
    {
		__u32 si_block[MAX_BLOCK_SIZE/sizeof(__u32)];
      
        fseek(fptr, (ext2_in.i_block[SINGLE_INDIRECT_BLOCK] * block_size ), SEEK_SET);
        fread((char*)&si_block, block_size, 1, fptr);

		for(i = 0; i < ( block_size / 4 ); i++)
		{
			char temp[MAX_BLOCK_SIZE];
			if(si_block[i] != 0 )
			{
				fseek(fptr, si_block[i] * block_size, SEEK_SET);
				fread((char*)temp, block_size, 1, fptr);
			}
			else
				memset(temp, 0, block_size);
            
			for(j = 0; j < min(block_size, bytecount); j++)
			{
				full_buf[bytepos++] = temp[j];
				hold.push_back(temp[j]);
				//printf("%c", temp[j]);
			}
        
			bytecount -= min(block_size, bytecount);

			if( !bytecount )
				break;      
		}    
    } 

    //Double Indirect Block
    if( bytecount )
    {
		__u32 di_block[MAX_BLOCK_SIZE/sizeof(__u32)];
      
      
		fseek(fptr, (ext2_in.i_block[DOUBLE_INDIRECT_BLOCK] * block_size ), SEEK_SET);
		fread((char*)&di_block, block_size, 1, fptr);

		for(i = 0; i < ( block_size / 4 ) && bytecount; i++)
		{

			__u32 temp[MAX_BLOCK_SIZE/sizeof(__u32)];

			fseek(fptr, di_block[i] * block_size, SEEK_SET);
			fread((char*)temp, block_size, 1, fptr);
          
			for(k = 0; k < block_size / 4; k++)
			{
				char block[MAX_BLOCK_SIZE];
				if(temp[k] != 0 )
				{
					fseek(fptr, temp[k] * block_size, SEEK_SET);
					fread((char*)block, block_size, 1, fptr);
				}
				else
					memset(block, 0, block_size);

				for(j = 0; j < min(block_size, bytecount); j++)
				{
					full_buf[bytepos++] = block[j];
					hold.push_back(temp[j]);
					//printf("%c", temp[j]);
				}
        
				bytecount -= min(block_size, bytecount);

				if( !bytecount )
					break;      
			}    
		} 
    }
    *ptr = full_buf;
	
	for(j = ext2_in.i_size; j < hold.size(); j++)
		{
			printf("%c", hold[j]);
		}
		printf("\n");
		
		
    return ext2_in.i_size;
}


__u32 print_data_3(int inumber, char** ptr)
{
    int i,j,k;
    int nblocks;
	std::vector<char> hold;
    char* full_buf;

    unsigned int bytecount = 0;
    unsigned int bytepos = 0;
    
    struct ext2_inode ext2_in = get_inode(inumber);

    bytecount = ext2_in.i_size;
    
    full_buf = (char*) malloc( bytecount * sizeof(char));
        
    nblocks = ( ( ext2_in.i_blocks * 512 ) / block_size);

    for(i = 0; i < DIRECT_DATA_BLOCKS; i++)
    {
		char temp[MAX_BLOCK_SIZE];
	
		if(ext2_in.i_block[i] != 0)
		{
			fseek(fptr, (ext2_in.i_block[i] * block_size ), SEEK_SET);
			fread((char*)&temp, block_size, 1, fptr);
		}
		else
			memset(temp, 0, block_size);

		for(j = 0; j < min(block_size, bytecount); j++)
		{
			full_buf[bytepos++] = temp[j];
			hold.push_back(temp[j]);
			//printf("%c", temp[j]);
		}
		bytecount -= min(block_size, bytecount);

		if( !bytecount )
			break;
    }      

    //Single Indirect Block
    if( bytecount )
    {
		__u32 si_block[MAX_BLOCK_SIZE/sizeof(__u32)];
      
        fseek(fptr, (ext2_in.i_block[SINGLE_INDIRECT_BLOCK] * block_size ), SEEK_SET);
        fread((char*)&si_block, block_size, 1, fptr);

		for(i = 0; i < ( block_size / 4 ); i++)
		{
			char temp[MAX_BLOCK_SIZE];
			if(si_block[i] != 0 )
			{
				fseek(fptr, si_block[i] * block_size, SEEK_SET);
				fread((char*)temp, block_size, 1, fptr);
			}
			else
				memset(temp, 0, block_size);
            
			for(j = 0; j < min(block_size, bytecount); j++)
			{
				full_buf[bytepos++] = temp[j];
				hold.push_back(temp[j]);
				//printf("%c", temp[j]);
			}
        
			bytecount -= min(block_size, bytecount);

			if( !bytecount )
				break;      
		}    
    } 

    //Double Indirect Block
    if( bytecount )
    {
		__u32 di_block[MAX_BLOCK_SIZE/sizeof(__u32)];
      
      
		fseek(fptr, (ext2_in.i_block[DOUBLE_INDIRECT_BLOCK] * block_size ), SEEK_SET);
		fread((char*)&di_block, block_size, 1, fptr);

		for(i = 0; i < ( block_size / 4 ) && bytecount; i++)
		{

			__u32 temp[MAX_BLOCK_SIZE/sizeof(__u32)];

			fseek(fptr, di_block[i] * block_size, SEEK_SET);
			fread((char*)temp, block_size, 1, fptr);
          
			for(k = 0; k < block_size / 4; k++)
			{
				char block[MAX_BLOCK_SIZE];
				if(temp[k] != 0 )
				{
					fseek(fptr, temp[k] * block_size, SEEK_SET);
					fread((char*)block, block_size, 1, fptr);
				}
				else
					memset(block, 0, block_size);

				for(j = 0; j < min(block_size, bytecount); j++)
				{
					full_buf[bytepos++] = block[j];
					hold.push_back(block[j]);
				}
        
				bytecount -= min(block_size, bytecount);

				if( !bytecount )
					break;      
			}    
		} 
    }
    *ptr = full_buf;
	
	
	if(hold.size() != 0)
	{
		printf("Inode %i\n", inumber);
		for(j = 0; j < hold.size(); j++)
		{
			printf("%x", hold[j]);
		}	
	}
    return ext2_in.i_size;
}

int main(int argc, char* argv[])
{
    struct ext2_super_block ext2_sb;

    fptr = fopen(argv[1], "r");
            
    // first, access the superblock
    fseek(fptr, SUPERBLOCK_OFFSET, SEEK_SET);

    fread((char*)&ext2_sb, sizeof(ext2_sb), 1, fptr); 

    total_blocks = ext2_sb.s_blocks_count;
    blocks_per_group = ext2_sb.s_blocks_per_group;    
    total_block_groups = (int)ceilf((float)total_blocks / (float)blocks_per_group);
    block_size = 1 << (10 + ext2_sb.s_log_block_size);
    inode_size = max( ext2_sb.s_inode_size, sizeof(struct ext2_inode));
    inodes_per_group = ext2_sb.s_inodes_per_group;

    fseek(fptr, (ext2_sb.s_first_data_block + 1) * block_size, SEEK_SET);

    for(int i = 0; i < total_block_groups; i++)
    {
        fread((char*)&ext2_gd[i], sizeof(struct ext2_group_desc), 1, fptr);
	}


	fprintf(stdout, "Missing Directory entries");
	fprintf(stdout, "\n********************************************************************************\n");
	
    char* token;
    int inode = 2;
	int length;
	
	char* data;
	length = print_data_2(inode, &data);
	fprintf(stdout, "Inode 2\n");
	if( !is_directory(inode) )
	{
		for(int i = 0; i < length; i++)
		{
			fprintf(stdout, "%c", data[i]);
		}
	}
	else
	{
		print_directory_2(inode, data, length, NULL);
		dir.erase (dir.begin(),dir.begin()+3);
	}
	fprintf(stdout, "\nContents of Inode %i\n",inode);
	for(int i = 0; i < length; i++)
		{
			fprintf(stdout, "%c ", data[i]);
		}
	
	
	while(!dir.empty())
	{
		char* data;
		inode = dir[dir.size()-1];
		
		fprintf(stdout, "\nInode %i\n",inode);
		dir.pop_back();
		length = print_data_2(inode, &data);
		print_directory_2(inode, data, length, NULL);
		dir.erase (dir.begin(),dir.begin()+2);
	
		fprintf(stdout, "\nContents of Inode %i\n",inode);
		for(int i = 0; i < length; i++)
			{
				fprintf(stdout, "%c ", data[i]);
			}
		files.push_back(inode);
	}
	fprintf(stdout, "\n\n********************************************************************************\n");
	fprintf(stdout, "\nFile Slack");
	fprintf(stdout, "\n********************************************************************************\n");
	
	for(int i = 0; i < files.size(); i++)
	{
		fprintf(stdout, "Inode %i\n", files[i]);
		length = print_data_2(files[i], &data);
		for(int j = length; j < sizeof(data); j++)
			{
				fprintf(stdout, "%x", data[j]);
			}
	}
	
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, "\nInode Data");
	fprintf(stdout, "\n********************************************************************************\n\n");
	
	for(int inode = 12; inode < ext2_sb.s_inodes_count; inode++)
	{
		char* data;
		bool match = false;
		for(int j = 0; j < files.size(); j++)
		{
			if(files[j] == inode)
			{
				match = true;
			}
		}
		if(match == false)
		{
			length = print_data_3(inode, &data);
		}
	}		
	
	fprintf(stdout, "\n\n********************************************************************************\n\n");
}