
#include "engio.h"


enum piece_id id_from_char(char pchar) {
  char piece_fen[6] = {'K', 'P', 'N', 'B', 'R', 'Q'};
  for (U8 i = 0; i < 6; i++) {
    if (pchar == piece_fen[i]) return (enum piece_id)i; 
    else if (pchar == piece_fen[i] - 'A' + 'a') return (enum piece_id)(i + 6); 
  }
  return NONE; 
}


U8 read_fen_castle(const char* castle, U8 start, U8 end) {

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


U8 sqr_to_internal(const char* sqr) {
  U8 rank = 8 - (sqr[1] - '0');  
  U8 file = sqr[0] - 97; 
  return rank * 8 + file; 
}


U32 move_to_internal(const char* move) {
    U8 origin = sqr_to_internal(move);
    U8 destination = sqr_to_internal(&move[2]); 
    return get_move(origin, destination, 0, 0);
}

U16 move_to_internal16(const char* move) {
    U8 origin = sqr_to_internal(move);
    U8 destination = sqr_to_internal(&move[2]); 
    return get_move16(origin, destination);
}
