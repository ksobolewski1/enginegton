
#pragma once

#include "core.h"


/*

Unit implementing the board 

*/


U8 get_move_count();


U32* get_move_list();


// builds the board from fen notation
U8 from_fen(const char* fen); 

// move generation function
U32* get_moves(enum board_state* s); 

