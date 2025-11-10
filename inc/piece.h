
#include "core.h"

#include "data.h"


/*

Piece attacks and moves

*/


struct piece {
    
    U8 sqr;
    enum piece_id id; 
}; 


inline U64 wpawn_att(const U8 sqr) {
    return wpawn_attacks[sqr];
}


inline U64 bpawn_att(const U8 sqr) {
    return bpawn_attacks[sqr];
}


inline U64 knight_att(const U8 sqr) {
    return knight_attacks[sqr];
}


inline U64 king_att(const U8 sqr) {
    return king_attacks[sqr];
}


U64 bishop_att(const U8 sqr, const U64 blockers_mask);


U64 rook_att(const U8 sqr, const U64 blockers_mask);


U64 wpawn_moves(const U8 sqr, const U64 en_passant_sqr, U64 blockers, const U64 opposing_pieces, U64 pin_path);


U64 bpawn_moves(const U8 sqr, const U64 en_passant_sqr, U64 blockers, const U64 opposing_pieces, U64 pin_path);


U64 knight_moves(const U8 sqr, const U64 own_pieces, U64 constraint);


U64 king_moves(const U8 sqr, const U64 own_pieces, const U64 attacks);


U64 bishop_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path);


U64 rook_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path);


U64 queen_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path);


U64 castle(U8 sqr, U8 rights, U64 board_mask, enum colour c, U64 attacks, U64 check);


