
#pragma once

#include "core.h"


struct mqueue; 
struct piece;


struct position {
    
    struct piece* board[64];
    
    struct piece* pawns[2]; 
	struct piece* hoppers[2]; // king and knights
    struct piece* sliders[2]; // rook, bishop, queen 
    
    U64 pin_paths[64];

    U64 side_masks[2];
    U64 board_mask;
    
    U8 en_passant_sqr;
    U8 castling_rights; 
    
    enum colour side;
    
};



void free_pos(struct position* pos); 


struct position* from_fen(char* fen); 


char* to_fen(struct position* pos); 


enum board_state get_moves(struct position* pos, struct mqueue* q); 
 

enum board_state update_pos(struct position* pos, struct mqueue* q, U32 move); 

