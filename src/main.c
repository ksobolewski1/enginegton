#include <stdio.h>


#include "engine.h"


#ifdef MAIN

int main(int argc, char** argv) {
    
    printf("The engine is on\n");
    return 0; 
}


#elif defined TEST


int main(int argc, char** argv) {

    if (argc < 3) {
        printf("Test name or test data were not passed to the engine. Exit.\n");
        return 1;
    }

    int test_code = *argv[1] - '0';

    switch (test_code) {
        case 0:
            if (generate_moves(argv[2]) == 1) {
                printf("Moge generation failed. Exit.\n");
            }
            break;
        default:
            printf("Test code not recognised by the engine. Exit.\n");
            return 1;
    }


    return 0;
}

#endif
