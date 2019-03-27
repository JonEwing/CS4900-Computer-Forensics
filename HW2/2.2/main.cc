//******************************************
//Jonathan Feige
//CS4900
//P2.2
//Takes the files as a path
//returns a dir or a file
//******************************************

#include <string.h>
#include <math.h>
#include <time.h> 
#include <vector>
#include <iostream>
#include "ext2fs.h"

#define MAX( a, b ) ( ( a > b) ? a : b )

using namespace std;

struct ext2_super_block super;
struct ext2_group_desc groupDec;
struct ext2_group_desc* gdptr;
std::vector<ext2_group_desc*> ext2_gd;
int size;
FILE *ptr;

	ext2_inode get_inode(int inumber,int block_size,int inode_size)
	{
		unsigned int i_group = ( inumber - 1 ) / super.s_inodes_per_group; 
		unsigned int i_index = ( inumber - 1 ) % super.s_inodes_per_group; 
		unsigned int i_offset = block_size * ext2_gd[i_group]->bg_inode_table + (i_index * inode_size); 
 
		struct ext2_inode ext2_in; 
		fseek(ptr, i_offset, SEEK_SET); 
		fread((char*)&ext2_in, sizeof(struct ext2_inode), 1, ptr); 
		return ext2_in;
	}

	void get_time(ext2_inode ext2_in)
	{
		time_t a_rawtime = ext2_in.i_atime; 
		char date[255]; // buffer to store time/date string 
		struct tm* a_time_tm; 
		
		a_time_tm = localtime(&a_rawtime); // convert from time_t to tm format 
		strftime(date, 255, "%b %d %R", a_time_tm); 
		printf("%s	", date); 
	}

	void get_user(ext2_inode ext2_in)
	{
		const unsigned int rights[9]={S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP,S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
		const char* crights[3] = {"r", "w", "x"};
		for(int i = 0; i < 9; i++)
		{
			if( ext2_in.i_mode & rights[i] )
			{
				printf("%s", crights[i % 3]);
			}
			else
			{
				printf("-");
			}
		}
	}


	int main(int argc, char *argv[])
	{	
		printf("File System %s\n", argv[1]);
		printf("Path %s\n", argv[2]);
		string path = argv[2];

		ptr = fopen(argv[1],"rb");  // r for read, b for binary

		if (ptr == NULL)
		{
			perror("Error while opening the file.\n");
			exit(EXIT_FAILURE);
		}
   
		unsigned char buffer[1024];
		
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
		super.s_blocks_count = (buffer[7]<<24) | (buffer[6]<<16)| (buffer[5]<<8) | buffer[4];
		super.s_first_data_block = (buffer[23]<<24) | (buffer[22]<<16)| (buffer[21]<<8) | buffer[20];
		super.s_blocks_per_group = (buffer[35]<<24) | (buffer[34]<<16)| (buffer[33]<<8) | buffer[32];
		super.s_inodes_per_group = (buffer[43]<<24) | (buffer[42]<<16)| (buffer[41]<<8) | buffer[40];
		
		int inode_size = MAX( super.s_inode_size, sizeof(struct ext2_inode));
		int block_size = 1 << (10 + super.s_log_block_size);
		int frag_size = 1 << (10 + super.s_log_frag_size);
		int num_block = floor(super.s_blocks_count/super.s_blocks_per_group);
		
		int group_block = (super.s_first_data_block + 1)*block_size;
		
		unsigned char block_buffer[block_size];
		
		fseek(ptr, group_block, SEEK_SET );	
		fread(block_buffer, block_size, 1, ptr);
		
		
		for (int i = 0; i <= num_block; i++)
		{	
			gdptr = (struct ext2_group_desc*) &block_buffer[i * sizeof(groupDec)];
			ext2_gd.push_back(gdptr);
		}
		
		int inumber = 2; //root
		char bbuf[block_size];
		char name[255];
		int i_index = (inumber - 1) % super.s_inodes_per_group;
		
		ext2_inode ext2_in;
		ext2_dir_entry_2 *dptr;
		ext2_dir_entry_2 d;
		
		ext2_in = get_inode(inumber,block_size,inode_size); //Get Inode data
		
		unsigned int i_offset = ext2_in.i_block[0] * block_size;
		
		
		fseek(ptr, i_offset, SEEK_SET);
		fread(bbuf, block_size, 1, ptr);

		if(path == "/")
		{
			for(int j = 0; j < block_size;)
			{	
				memcpy(&d,&bbuf[j],sizeof(d));

				get_user(ext2_in);
				
				ext2_in = get_inode(d.inode,block_size,inode_size);
				
				printf("	%d",ext2_in.i_uid);
				printf("	%d	",ext2_in.i_size);
				get_time(ext2_in);

				memcpy(name,d.name,d.name_len);
				name[d.name_len] = '\0';

				printf("%s\n",name);
		
				j = j + d.rec_len;
			}
			return -1;
		}
		
		std::string delimiter = "/";
		vector<string> tok;

		size_t pos = 0;
		std::string token;
		while ((pos = path.find(delimiter)) != std::string::npos) 
		{
    		token = path.substr(0, pos);
			tok.push_back(token);
    		path.erase(0, pos + delimiter.length());
		}

		tok.push_back(path);

		tok.erase (tok.begin());
		tok.shrink_to_fit();
		
		bool found_next = false;
		
			while(!tok.empty())
			{
				for(int j = 0; j < block_size;)
				{	
					memcpy(&d,&bbuf[j],sizeof(d));
					memcpy(name,d.name,d.name_len);
					name[d.name_len] = '\0';

					if(tok[0]==name)
					{
						found_next = true;
						inumber = d.inode;
					}
					j = j + d.rec_len;
				}
				
				if (tok[0].find(".") != std::string::npos && found_next == true) 
				{
					ext2_in = get_inode(inumber,block_size,inode_size); //Get Inode 
					size = ext2_in.i_size;
					for(int i = 0; i < 12; i++)
					{
						unsigned int i_offset = ext2_in.i_block[i] * block_size;
						fseek(ptr, i_offset, SEEK_SET);
						fread(bbuf, block_size, 1, ptr);
						
						if(ext2_in.i_block[i] != 0)
						{
							for(int j = 0; j < block_size; j++)
							{
								size--;
								if(size >= 0)
								{
									printf("%c",bbuf[j]);
								}
							}
						}
					}
				}
				
				if(ext2_in.i_block[12] != 0) // single indirect
				{
					fseek(ptr, ext2_in.i_block[12] * block_size, SEEK_SET);
					fread(bbuf, block_size, 1, ptr);
					unsigned int bp[block_size/4];
						
					int j = 0;
					for(int i = 0; i < block_size/4; i = i + 2)
					{
						bp[j] = (bbuf[i + 1]<<8) | bbuf[i];
						j++;
					}
					
					for(int i = 0; i < block_size/4; i = i + 2)
					{
						fseek(ptr, bp[i] * block_size, SEEK_SET);
						fread(bbuf, block_size, 1, ptr);
						
						for(int j = 0; j < block_size; j++)
						{
							size--;
							if(size >= 0)
							{
								printf("%c",bbuf[j]);
							}
						}
					}
				}
					
				if(ext2_in.i_block[13] != 0) // single indirect
				{
					fseek(ptr, ext2_in.i_block[12] * block_size, SEEK_SET);
					fread(bbuf, block_size, 1, ptr);
					unsigned int bp[block_size/4];
					unsigned int bp_2[block_size/4];
					
					int num = 0;
					for(int i = 0; i < block_size/4; i = i + 2)
					{
						bp[num] = (bbuf[i + 1]<<8) | bbuf[i];
						num++;
					}
						
					for(int i = 0; i < block_size/4; i++)
					{
						fseek(ptr, bp[i] * block_size, SEEK_SET);
						fread(bbuf, block_size, 1, ptr);
								
						for(int j = 0; i < block_size/4; j = j + 2)
						{
							bp_2[num] = (bbuf[i + 1]<<8) | bbuf[i];
							num++;
						}
	
						for(int j = 0; i < block_size/4; j = j + 2)
						{
							fseek(ptr, bp_2[i] * block_size, SEEK_SET);
							fread(bbuf, block_size, 1, ptr);
					
							for(int j = 0; j < block_size; j++)
							{
								size--;
								if(size >= 0)
								{
									printf("%c",bbuf[j]);
								}
							}
						}
					}
				}
				
				else if(tok.size() == 1)
				{
					ext2_in = get_inode(inumber,block_size,inode_size);
					
					fseek(ptr, i_offset, SEEK_SET);
					fread(bbuf, block_size, 1, ptr);
					
					for(int j = 0; j < block_size;)
					{	
						memcpy(&d,&bbuf[j],sizeof(d));
						
						ext2_in = get_inode(d.inode,block_size,inode_size);
						
						get_user(ext2_in);
						printf("	%d",ext2_in.i_uid);
						printf("	%d	",ext2_in.i_size);
						get_time(ext2_in);

		
						memcpy(name,d.name,d.name_len);
						name[d.name_len] = '\0';

						printf("%s\n",name);
			
						j = j + d.rec_len;
					}
				}
				
				else
				{
					ext2_in = get_inode(inumber,block_size,inode_size);
					
					fseek(ptr, i_offset, SEEK_SET);
					fread(bbuf, block_size, 1, ptr);
				}
				
				if(found_next == false)
				{
					cout<<"Invalid Path\n";
					return -1;
				}
				
				tok.erase (tok.begin());
				tok.shrink_to_fit();		 
			}
	}