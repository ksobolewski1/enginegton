#include "engine.h"

#include "core.h"
#include "pos.h"
#include "mqueue.h"


#ifdef TEST

#include "debug.h"

#endif


void generate_moves(const char* fen) {

    struct position* pos = from_fen(fen);
    struct mqueue* q = get_mqueue();

    enum board_state state = get_moves(pos, q);

    printf("%i", q->N);
}