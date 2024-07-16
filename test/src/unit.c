
#include "unit.h"

#include "helpers.h" 

#include "engio.h" 
#include "pos.h"
#include "moves.h"
#include "mqueue.h"

#include <stdio.h>
#include <stdlib.h>


void free_all(char** in, struct mqueue** exp, U32 n) {
    for (U32 i = 0; i < n; i++) {
	free(in[i]); free(exp[i]);
    }
    free(in); free(exp); 
}

enum test_code moves_test(const char* inf, const char* outf) {

    // hard-coded? 
    U32 line_count = 5; 
    char** in = read_in(inf, line_count);

    // create move queues 
    struct mqueue** expects = (struct mqueue**)malloc(sizeof(struct mqueue*) * (line_count + 1));
    if (expects == NULL || in == NULL) {
	fprintf(stderr, "Error: Failed to allocate memory for in/out data in moves_test\n");
	return FAILURE; 
    }

    // initialise the move queues  
    for (U32 i = 0; i < line_count; i++) {
	expects[i] = get_mqueue();
	if (expects[i] == NULL) {
	    fprintf(stderr, "Error: Failed to allocate memory for an expected output queue in moves_test\n");
	    return FAILURE;
	}
    }

    // fill up the move queues for the expected output 
    if (read_moves(outf, expects) == FAILURE) return FAILURE; 

    // for each line of fen:
    //     intiialise a position and move queue
    //     get moves and compare to the expected output 
    for (U32 i = 0; i < line_count; i++) {

	struct position* pos = from_fen(in[i]);
	if (pos == NULL) {
	    fprintf(stderr, "Error: Failed to allocate memory for a position in moves_test\n");
	    return FAILURE;
	}
	struct mqueue* q = get_mqueue();
	if (q == NULL) {
	    fprintf(stderr, "Error: Failed to allocate memory for a queue in moves_test\n");
	    return FAILURE; 
	}
	
	get_moves(pos, q);

	if (verify_moves(q, expects[i]) == FAILURE) {
	    free_pos(pos); free(q); free_all(in, expects, line_count); 
	    return FAILURE; 
	}
	
	free_pos(pos);
	free(q); 
	
    }
    
    free_all(in, expects, line_count); 
    return SUCCESS;
    
}
