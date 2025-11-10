
#pragma once

#include "core.h"


/*

Functions that facilitate data exchange with other programs, notably by translating to/from uci

*/

enum piece_id id_from_char(char pchar); 


U8 castle_from_fen(const char* castle, U8 start, U8 end);


U8 sqr_from_uci(const char* sqr); 


const char* sqr_to_uci(U8 sqr);


U32 move_from_uci(const char* move);


U16 move_from_uci16(const char* move);


const char* move_to_uci(U32 move);


// returns moves separated with ';'
const char* moves_to_uci(U32* moves, U8 move_count);