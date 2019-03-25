//******************************************
//Jonathan Feige
//CS4900
//P1
//Takes a binary file and resturs a TMUFS
//******************************************

#include<stdio.h>
#include <stdlib.h>

#define TMUFS_FN_LENGTH 10
#define TMUFS_ATTRIBUTE_ASCII    0x01
#define TMUFS_ATTRIBUTE_BINARY   0x02
#define TMUFS_ATTRIBUTE_HIDDEN   0x04
#define TMUFS_ATTRIBUTE_DELETED  0x08
	
	int main()
	{	
		char file_name[16];
		FILE *ptr;
		
		printf("Enter name of a file you wish to see\n");
		gets(file_name);
	
		ptr = fopen(file_name,"rb");  // r for read, b for binary

		if (ptr == NULL)
		{
			perror("Error while opening the file.\n");
			exit(EXIT_FAILURE);
		}
   
		unsigned char buffer[32];
		unsigned char file_buffer[40];
		
		fread(buffer,sizeof(buffer),1,ptr);
		
		unsigned int result = ((buffer[1]<<8) | buffer[0]);
		if (result != 4660)
		{
			printf("wrong magic number\n");
			return -1;
		}
		
		printf("\nTMUFS HEADER\n---------------------------------------------------------------------------");
		
		printf("\nMagic Number: ");
		printf("%x\n", result);
	
		result = (buffer[11]<<24) | (buffer[10]<<16)| (buffer[9]<<8) | buffer[8];
		unsigned int file_ammout = result;
		printf("Number of Directory Entries: ");
		printf("%d\n", result);
		
		result = (buffer[19]<<24) | (buffer[18]<<16)| (buffer[17]<<8) | buffer[16];
		printf("Byte Offset of Data Area in %s:",file_name);
		printf("%d\n",result);
		
		result = (buffer[27]<<24) | (buffer[26]<<16)| (buffer[25]<<8) | buffer[24];
		printf("Checksum of the Dara Area: ");
		printf("%d\n",result);
		
		printf("---------------------------------------------------------------------------\nFile No.	File Name	Size	Start Offset	Attributes\n---------------------------------------------------------------------------\n");
		
			unsigned int  size[1000];
			unsigned int  type[1000];
			unsigned char stuff[1000000];

			for (int i = 0; i < file_ammout; i++)
			{
				fread(file_buffer,sizeof(file_buffer),1,ptr);
		
				printf("%d		", i+1);
			
				for (int i = 23; i < 33; i++)
				{
				printf("%c",file_buffer[i]);
				}
				
				result = (file_buffer[3]<<24) | (file_buffer[2]<<16)| (file_buffer[1]<<8) | file_buffer[0];
				size[i] = result;
				printf("	%d", result);
		
				result = (file_buffer[19]<<24) | (file_buffer[18]<<16)| (file_buffer[17]<<8) | file_buffer[16];
				printf("	%d", result);
				
				if( file_buffer[8] & TMUFS_ATTRIBUTE_ASCII)
				{
					type[i] = 1;
					printf("		ASCII\n");
				}
				else if (file_buffer[8] & TMUFS_ATTRIBUTE_BINARY)
				{
					type[i] = 2;
					printf("		BINARY\n");
				}
				else
				{
					printf("Error reading files\n");
					return -1;
				}
			}
			
			
			for( int i = 0; i < file_ammout; i++)
			{
				printf("---------------------------------------------------------------------------\nContents of File %d\n---------------------------------------------------------------------------\n",i + 1);
		
				fread(stuff,size[i],1,ptr);
				if(type[i] == 1)
				{
					for (int x = 0; x < size[i]; x++)
					{
						printf("%c",stuff[x]);
					}
				}
				if(type[i] == 2)
					{
					printf("[HEXDUMP]\n");
					for (int x = 0; x < size[i]; x++)
					{
						printf("%x ",stuff[x]);
					}
				}
				printf("\n");
				}
			printf("---------------------------------------------------------------------------\n");	
	}
