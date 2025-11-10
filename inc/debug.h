#pragma once

#include <stdio.h>

/*

    Functions to help debug the program

*/

static inline void printU64(U64 data) {

    printf("U64: %llu\n", data);
    
    for (unsigned int i = 0; i < 64; i++) {

	if ((data & (1ULL << i)) > 0) printf(" 1 ");
	else printf(" O ");

	if (i % 8 == 7) printf("\n");
    }

    printf("\n");
}


static inline void print_move(U32 move) {

    U64 board = 0ULL;
    board = (1ULL << origin(move)) | (1ULL << destination(move));
    printU64(board);
}