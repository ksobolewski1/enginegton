
#include "core.h"

struct position;

struct piece {

    struct piece* prev;
    struct piece* next;
    
    U8 sqr;
    enum piece_id id; 
    
};


struct piece* get_piece(enum piece_id id, U8 sqr);

void free_piece(struct piece* p);
