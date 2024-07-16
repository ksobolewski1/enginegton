
#pragma once

#include "core.h"

struct mqueue; 

char** read_in(const char* filename, int line_count); 

U8 read_moves(const char* filename, struct mqueue** expects); 

U8 verify_moves(struct mqueue* m, struct mqueue* expect); 
