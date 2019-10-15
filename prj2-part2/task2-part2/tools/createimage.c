#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int kernelsize;

struct ph {
	uint32_t	offset;
	uint32_t	filesz;
	uint32_t	memsz;
} boot_phtable[16], kernel_phtable[16], newboot_phtable[16];


int main()
{
	FILE * kernel = fopen("main", "r");
	FILE * bootblock = fopen("bootblock", "r");
	
	uint16_t boot_ph_num;
	uint16_t kernel_ph_num;
	
	printf("Reading files...\n");
	//begin reading boot
	printf("Begin reading bootblock:\n");	
	fseek(bootblock, 0x2c, SEEK_SET);
	fread(&boot_ph_num, sizeof(uint16_t), 1, bootblock);
	printf("bootblock:\tThe number of program header(s):%d\n", boot_ph_num);
	//Move to the program header(s).
	fseek(bootblock, 0x34, SEEK_SET);
	int temp_boot_num = 0;
	
	printf("bootblock:\tBegin reading program header(s):\n");
	
	while(temp_boot_num < boot_ph_num) {
		//offset
		printf("bootblock:\tReading program header(s): %d/%d\n", temp_boot_num+1, boot_ph_num);
		fseek(bootblock, 0x04, SEEK_CUR);
		fread(&(boot_phtable[temp_boot_num].offset), sizeof(uint32_t), 1, bootblock);
		//filesize
		fseek(bootblock, 0x08, SEEK_CUR);
		fread(&(boot_phtable[temp_boot_num].filesz), sizeof(uint32_t), 1, bootblock);
		//memsize
		fread(&(boot_phtable[temp_boot_num].memsz), sizeof(uint32_t), 1, bootblock);
		//goto next block begin
		fseek(bootblock, 0x08, SEEK_CUR);
		printf("bootblock:\t%d:\toffset:%x\tfile size:%dB\tmemory size:%dB\n", temp_boot_num + 1, boot_phtable[temp_boot_num].offset, boot_phtable[temp_boot_num].filesz, boot_phtable[temp_boot_num].memsz);
		temp_boot_num++;
	}
	printf("bootblock:\tDone.\nReading bootblock done.\n");
	
	//begin kernel
	printf("Begin reading kernel:\n");	
	fseek(kernel, 0x2c, SEEK_SET);
	fread(&kernel_ph_num, sizeof(uint16_t), 1, kernel);
	printf("kernel:\tThe number of program header(s):%d\n", kernel_ph_num);
	//Move to the program header(s).
	fseek(kernel, 0x34, SEEK_SET);
	int temp_kernel_num = 0;
	
	printf("kernel:\tBegin reading program header(s):\n");
		
	while(temp_kernel_num < kernel_ph_num) {
		//offset
		printf("kernel:\tReading program header(s): %d/%d\n", temp_kernel_num+1, kernel_ph_num);
		fseek(kernel, 0x04, SEEK_CUR);
		fread(&(kernel_phtable[temp_kernel_num].offset), sizeof(uint32_t), 1, kernel);
		//filesize
		fseek(kernel, 0x08, SEEK_CUR);
		fread(&(kernel_phtable[temp_kernel_num].filesz), sizeof(uint32_t), 1, kernel);
		//memsize
		fread(&(kernel_phtable[temp_kernel_num].memsz), sizeof(uint32_t), 1, kernel);
		//goto next block begin
		fseek(kernel, 0x08, SEEK_CUR);
		printf("kernel:\t%d:\toffset:%x\tfile size:%dB\tmemory size:%dB\n", temp_kernel_num = 1, kernel_phtable[temp_kernel_num].offset, kernel_phtable[temp_kernel_num].filesz, kernel_phtable[temp_kernel_num].memsz);
		temp_kernel_num++;
	}
	printf("kernel:\tDone.\nReading kernel done.\n");

	printf("Reading files done.\n\n");
	
	printf("Begin writing image...\n");
	FILE *image = fopen("image", "w");

	
	printf("Begin writing bootblock...\n");
	//bootblock

	for(int i = 0; i < boot_ph_num; i++) {
		printf("bootblock:\tWriting program block(s):%d/%d\n", i + 1, boot_ph_num);
		fseek(bootblock, boot_phtable[i].offset, SEEK_SET);
		uint8_t temp_char;		
		for(int j = 0; j < boot_phtable[i].filesz; j++) {
			fread(&temp_char, sizeof(uint8_t), 1, bootblock);
			fwrite(&temp_char, sizeof(uint8_t), 1, image);
		}
		printf("bootblock:\tPadding the remaining memory size with 0...\n");
		for(int j = 0; j < boot_phtable[i].memsz - boot_phtable[i].filesz; j++) {
			uint8_t num_zero = 0;
			fwrite(&num_zero, sizeof(uint8_t), 1, image);
		}
		printf("bootblock:\tPadding the remaining block with 0...\n");
		for(int j = 0; j < 504 - boot_phtable[i].memsz; j++) {
			uint8_t num_zero = 0;
			fwrite(&num_zero, sizeof(uint8_t), 1, image);
		}
		kernelsize = 0;
		while(kernelsize < kernel_phtable[i].memsz) {
			kernelsize += 512;
		}
		printf("bootblock:\tWrinting the OS size...\n");
		fwrite(&kernelsize, sizeof(uint32_t), 1, image);
				
		printf("bootblock:\tWriting the end of bootblock...\n");
		uint32_t temp_num = 0xaa550000;
		fwrite(&temp_num, sizeof(uint32_t), 1, image);
	}
	printf("bootblock:\tWriting done.\n");
	
	
	
	//kernel
	for(int i = 0; i < kernel_ph_num; i++) {
		fseek(kernel, kernel_phtable[i].offset, SEEK_SET);
		uint8_t temp_char;		
		for(int j = 0; j < kernel_phtable[i].filesz; j++) {
			fread(&temp_char, sizeof(uint8_t), 1, kernel);
			fwrite(&temp_char, sizeof(uint8_t), 1, image);
		}
		for(int j = 0; j < kernel_phtable[i].memsz - kernel_phtable[i].filesz; j++) {
			uint8_t num_zero = 0;
			fwrite(&num_zero, sizeof(uint8_t), 1, image);
		}
		
		for(int j = 0; j < kernelsize - kernel_phtable[i].memsz; j++) {
			uint8_t num_zero = 0;
			fwrite(&num_zero, sizeof(uint8_t), 1, image);
		}
	}
	printf("kernel:\t\tWriting done.\n");

	printf("Image writing done.\n\n");
	fclose(bootblock);
	fclose(kernel);

	fclose(image);
	printf("Done.\n");
	
	return 0;
}
