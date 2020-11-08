#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	FILE *inputFile, *outputFile;
	unsigned short magic;
	unsigned int version, chunk_list_len, max_chunk_size;
	unsigned int *chunks_list;
	unsigned char *chunk_ptr;
	
	if (3 != argc) {
		printf("Usage: %s <input_grt_file> <output_file>\n", argv[0]);
		return -EINVAL;
	}

	inputFile = fopen(argv[1], "r");
	if (NULL == inputFile) {
		perror("Failed to open input file: ");
		return errno;
	}

	outputFile = fopen(argv[2], "w");
	if (NULL == outputFile) {
		perror("Failed to open output file: ");
		return errno;
	}

	fread(&magic, sizeof magic, 1, inputFile);
	if (0x5447 != magic) {
		printf("Bad magic in header.\n");
		return -EINVAL;
	}

	fread(&version, sizeof version, 1, inputFile);
	if (1 != version) {
		printf("Bad version in header.\n");
		return -EINVAL;
	}

	fread(&chunk_list_len, sizeof chunk_list_len, 1, inputFile);
	if (0 >= chunk_list_len) {
		printf("Bad chunk list length in header.\n");
		return -EINVAL;
	}

	printf("GRT File info:\n\tMagic: 0x%x\n\tVersion: 0x%x\n\tNum chunks: 0x%x\n", magic, version, chunk_list_len);
	chunks_list = (unsigned int*)malloc((chunk_list_len * sizeof(unsigned int)) * 2);
	if (NULL == chunks_list) {
		printf("Failed to allocate %d bytes for chunk list.\n", chunk_list_len*sizeof(unsigned int)*2);
		return -ENOMEM;
	}
	
	printf("Reading in chunk list... ");
	max_chunk_size = 0;
	for (int i=0; i<=(chunk_list_len*2)-2 ;i+=2) {
		fread(chunks_list+i, sizeof(unsigned int), 1, inputFile);
		fread(chunks_list+i+1, sizeof(unsigned int), 1, inputFile);
		if (*(chunks_list+i+1) > max_chunk_size)
			max_chunk_size = *(chunks_list+i+1);
	}
	printf("Done\nMax chunk size is %d\n", max_chunk_size);

	chunk_ptr = (unsigned char*)malloc(max_chunk_size * sizeof(unsigned char));
	if (NULL == chunk_ptr) {
		printf("Failed to allocate %d bytes for chunk data.\n", sizeof(unsigned char)*max_chunk_size);
		return -ENOMEM;
	}

	for (int i=0; i<=chunk_list_len*2-2; i+=2) {
		printf("\rConverting chunk %d (%d/%d)", *(chunks_list+i),(i/2)+1 ,chunk_list_len);
		fseek(inputFile, *(chunks_list+i), SEEK_SET);
		fread(chunk_ptr, *(chunks_list+i+1), sizeof(unsigned char), inputFile);
		for(int j=0; j<=*(chunks_list+i+1); j++) {
			chunk_ptr[j] = chunk_ptr[j] ^ 0xff;
		}
		fwrite(chunk_ptr, sizeof(unsigned char), *(chunks_list+i+1), outputFile);
	}
	printf(" Done\nConversion finished.\n");

	free(chunks_list);
	free(chunk_ptr);
	fclose(inputFile);
	fclose(outputFile);
		
	return 0;
}
