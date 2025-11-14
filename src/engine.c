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
        printf("Failed to load the position from fen.\n");
        return 1;
    }
    
    enum board_state s;
    get_moves(&s);

    const char* res = pos_to_uci(get_move_list(), get_move_count(), s);
    if (!res) {
        printf("Failed to convert moves to uci.\n");
        return 1;
    }
    printf(res);
    return 0;

}