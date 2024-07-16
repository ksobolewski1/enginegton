#include <stdio.h>

#ifdef MAIN

int main(int argc, char** argv) {
    
    printf("The engine is on\n");
    return 0; 
}


#elif defined TEST

#include "pos.h"
#include "moves.h"
#include "mqueue.h"

#include "unit.h"

#include <string.h>

int main(int argc, char** argv) {

    // argv[1] is the root directory
    // argv[2] is the name of the test 
    
    enum test_code res = SUCCESS; 

    struct position* pos = from_fen("8/8/3k4/8/3K4/8/8/8 w - - 0 1");
    struct mqueue* q = get_mqueue();
    get_moves(pos, q);

    /* switch (id) { */
    /* case BASE: */
    /* 	res = moves_test(); */
    /* 	break;  */
    /* } */

    // individual tests responsible for printing out the diffs and/or times, and any other useful information

    if (res == SUCCESS) printf("\e[32mSUCCESS\e[0m\n");
    else printf("\e[33mFAILED\e[0m\n");
    
    return res;
}

#endif
