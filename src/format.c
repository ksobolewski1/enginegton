
#include "format.h"

#include <stdio.h>
#include <string.h>


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
}


const char** queue_to_uci(struct mqueue* q) {

    
}