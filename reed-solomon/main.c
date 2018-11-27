#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rs.h"
#include <sys/random.h>

struct config{
	int num_data;
	int num_entropy;
	int num_carrier;
	int polynomial_deg;
	int k;
	int n;
	int total_blocks;
	int encode_blocks;
	int block_portion;
	int padding;
	int block_size;
};

void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

int initialize(struct config* configuration, int num_data, int num_entropy, int num_carrier){
	configuration->total_blocks = num_data + num_entropy + num_carrier;
	configuration->num_data = num_data;
	configuration->num_entropy = num_entropy;
	configuration->num_carrier = num_carrier;
	configuration->padding = 255 % configuration->total_blocks;
	configuration->block_portion = 255 / configuration->total_blocks;
	configuration->n = configuration->total_blocks * configuration->block_portion;
	configuration->k = configuration->block_portion * (num_data + num_entropy);
	configuration->block_size = 4096;
	configuration->encode_blocks = configuration->block_size / configuration->block_portion;
	if ((configuration->block_size % configuration->block_portion) != 0){
		configuration->encode_blocks++;
	}
	init_rs(configuration->k);
	return 0;
}

int encode(struct config* info, unsigned char* data, unsigned char* entropy, unsigned char* carrier){
	int i, j;
	int count = 0;
	int data_count = 0;
	int entropy_count = 0;
	int carrier_count = 0;
	unsigned char* encode_buffer = malloc(255);
	for(i = 0; i < info->encode_blocks; i++){
		for(j = 0; j < info->num_data; j++){
			memcpy(&encode_buffer[count], &data[data_count], info->block_portion);
			count += info->block_portion;	
			data_count += info->block_portion;
		}
		for(j = 0; j < info->num_entropy; j++){
			memcpy(&encode_buffer[count], &entropy[entropy_count], info->block_portion);
			count += info->block_portion;
			entropy_count += info->block_portion;
		}
		encode_rs(encode_buffer, info->k, &encode_buffer[info->k], 255-info->k);
		memcpy(&carrier[carrier_count], &encode_buffer[info->k], info->num_carrier * info->block_portion);
	}
	free(encode_buffer);
	return 0;
}


//if a block is all zeros then it is considered erased
int decode(struct config* info, int erasures, int* err_loc, unsigned char* data, unsigned char* entropy, unsigned char* carrier){
	int i, j;
	int count = 0;
	int data_count = 0;
	int entropy_count = 0;
	int carrier_count = 0;
	unsigned char* decode_buffer = malloc(255);
	for(i = 0; i < info->encode_blocks; i++){
		for(j = 0; j < info->num_data; j++){

		}
		//TODO only read in the entropy if needed and supplied
		//TODO is it faster to read the entropy or reconstruct more of the data?
		for(j = 0; j < info->num_entropy; j++){

		}
		//new stuff
		for(j = 0; j< info->num_data; j++){

		}
	}
	free(encode_buffer);
	return 0;
}

int main(void){
	//initialize the variables
	init_rs(223);
	unsigned char *data = malloc(255);
	getrandom(data, 128, 0);
	hexDump("data", data, 255);
	encode_rs(data, 223, &data[223], 32);
	hexDump("Encoded data", data, 255);
	memset(data, 0, 6);
	hexDump("erased data", data, 255);
	int *erased_pos = malloc(6);
	for(int i = 0; i < 6; i++){
		erased_pos[i] = i;
	}
	int errs = eras_dec_rs(data, erased_pos, 223, 6);
	printf("Errors: %d\n", errs);
	hexDump("decoded data", data, 255);
	return 0;
}
