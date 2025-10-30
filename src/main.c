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
        printf("Test name or test data were not passed to the engine. Exit.");
        return 1;
    }

    int test_code = *argv[1] - '0';

    switch (test_code) {
        case 0:
            generate_moves(argv[2]);
            break;
        default:
            printf("Test code not recognised by the engine.");
            return 1;
    }


    return 0;
}

#endif
