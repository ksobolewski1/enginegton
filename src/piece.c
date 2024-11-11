
#include "piece.h"

#include <stdio.h>
#include <stdlib.h>


struct piece* get_piece(enum piece_id id, U8 sqr) {

    struct piece* p = (struct piece*)malloc(sizeof(struct piece));

    if (p == NULL) {
	fprintf(stderr, "Error: Failed to allocate memory for struct 'piece'\n");
	return NULL;
    }

    p->attacks = p->moves = 0; 
    p->id = id; 
    p->sqr = sqr; 
    p->next = p->prev = NULL; 

    return p; 
}


void free_piece(struct piece* p) {
    if (p->next != NULL) free_piece(p->next);
    p->next = p->prev = NULL; 
    free(p); 
}


