
#pragma once

#include "core.h"


struct mqueue; 
struct piece;


struct position {
    
    //U64 hop_attacks[2];
    //U64 slide_attacks[2]; 
    
    struct piece* board[64];
    
    U64 pin_paths[64];

    struct piece* hoppers[2]; 
    struct piece* sliders[2]; 

    U64 side_masks[2];
    U64 board_mask;
    
    U8 en_passant_sqr;
    U8 castling_rights; 
    
    enum colour side;
    
};



void free_pos(struct position* pos); 


struct position* from_fen(char* fen); 


char* to_fen(struct position* pos); 


