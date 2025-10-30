#pragma once

#include <stdio.h>


static inline void printU64(U64 data) {

    printf("U64: %llu\n", data);
    
    for (unsigned int i = 0; i < 64; i++) {

	if ((data & (1ULL << i)) > 0) printf(" 1 ");
	else printf(" O ");

	if (i % 8 == 7) printf("\n");
    }

    printf("\n");
}
