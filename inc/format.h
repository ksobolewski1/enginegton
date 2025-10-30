
#pragma once

#include "core.h"

#include <stdio.h>

/*

   

*/ 


enum piece_id id_from_char(char pchar); 

U8 read_fen_castle(const char* castle, U8 start, U8 end);

U8 sqr_to_internal(const char* sqr); 

U32 move_to_internal(const char* move);

U16 move_to_internal16(const char* move);
