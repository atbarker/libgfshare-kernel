#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rs.h"
#include <sys/random.h>

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

int main(void){
	//initialize the variables
	init_rs();
	unsigned char *data = malloc(255);
	getrandom(data, 192, 0);
	hexDump("data", data, 255);
	encode_rs(data, &data[192]);
	hexDump("Encoded data", data, 255);
	memset(data, 0, 10);
	hexDump("erased data", data, 255);
	int errs = eras_dec_rs(data, NULL, 0);
	printf("Errors: %d\n", errs);
	hexDump("decoded data", data, 255);
	return 0;
}
