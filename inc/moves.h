
#pragma once 

#include "core.h"


struct position;
struct mqueue; 


enum board_state get_moves(struct position* pos, struct mqueue* q); 
 

enum board_state update_pos(struct position* pos, struct mqueue* q, U32 move); 

