#include <stdio.h>

#ifdef MAIN

int main(int argc, char** argv) {
    
    printf("The engine is on\n");
    return 0; 
}


#elif defined TEST

int main(int argc, char** argv) {

    return 0;
}

#endif
