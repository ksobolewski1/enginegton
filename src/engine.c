#include "engine.h"

#include "core.h"
#include "pos.h"
#include "format.h"

#include <stdio.h>

#ifdef TEST

#include "debug.h"

#endif


int generate_moves(const char* fen) {

    if (from_fen(fen)) {
        printf("Failed to load in the position from fen");
        return 1;
    }
    
    get_moves();

    const char* res = moves_to_uci(get_move_list(), get_move_count());
    if (!res) {
        printf("Failed to convert moves to uci. Exit");
        return 1;
    }
    printf(res);
    return 0;

}