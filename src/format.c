
#include "format.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef TEST 

#include "debug.h"
#include "data.h"

#endif

enum piece_id id_from_char(char pchar) {
  char piece_fen[6] = {'K', 'P', 'N', 'B', 'R', 'Q'};
  for (U8 i = 0; i < 6; i++) {
    if (pchar == piece_fen[i]) return (enum piece_id)i; 
    else if (pchar == piece_fen[i] - 'A' + 'a') return (enum piece_id)(i + 6); 
  }
  return NONE; 
}


U8 castle_from_fen(const char* castle, U8 start, U8 end) {

  U8 res = 0; 
  for (U8 i = start; i < end; i++) {
    if (castle[i] == 'K') res += 1; 
    else if (castle[i] == 'Q') res += 2; 
    else if (castle[i] == 'k') res += 4; 
    else if (castle[i] == 'q') res += 8; 
    else printf("Unrecognised symbol in castling rights. Ignored.\n");
  }
  return res; 

}


U8 sqr_from_uci(const char* sqr) {
  U8 rank = 8 - (sqr[1] - '0');  
  U8 file = sqr[0] - 97; 
  return rank * 8 + file; 
}

const char* sqr_to_uci(U8 sqr) {

  U8 file = sqr % 8;
	U8 rank = 8 - ((sqr - file) >> 3);

  char* csqr = malloc(3);
  if (!csqr) {
        printf("Malloc fail at sqr_to_uci.\n");
        return NULL;
  }

  csqr[0] = file + 'a';
  csqr[1] = rank + '0';
  csqr[2] = '\0';

  return csqr;

}


U32 move_from_uci(const char* move) {
    U8 origin = sqr_from_uci(move);
    U8 destination = sqr_from_uci(&move[2]); 
    return get_move(origin, destination, 0, 0);
}


U16 move_from_uci16(const char* move) {
    U8 origin = sqr_from_uci(move);
    U8 destination = sqr_from_uci(&move[2]); 
    return get_move16(origin, destination);
}


const char* move_to_uci(U32 move) {
    U8 o = origin(move);
    U8 d = destination(move);

    char* cmove = malloc(5);
    if (!cmove) {
      printf("Malloc fail at move_to_uci\n");
    }
    cmove[0] = '\0';
    strcat(cmove, sqr_to_uci(o));
    strcat(cmove, sqr_to_uci(d));

    return cmove;
}


const char* pos_to_uci(U32* moves, U8 moves_count, enum board_state s) {

  char* res = malloc(sizeof(char) * ((moves_count * 4) + (moves_count + 1)));
  if (!res) {
    printf("Malloc fail in moves_to_uci.\n");
  }
  res[0] = '\0';

  for (U8 i = 0; i < moves_count; i++) {
    const char* m = move_to_uci(moves[i]);
    if (!m) return NULL;
    strcat(res, m);
    strcat(res, ";");
  }

  char state[2];
  state[0] = (char)s + '0';
  state[1] = '\0';
  strcat(res, state);

  return res;
}