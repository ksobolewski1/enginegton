#pragma once

#include "data.h"


struct mqueue;


// move and attack functions 


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


void wpawn_moves(const U8 sqr, const U64 en_passant_sqr, U64 blockers, const U64 opposing_pieces, U64 pin_path, struct mqueue* q);


void bpawn_moves(const U8 sqr, const U64 en_passant_sqr, U64 blockers, const U64 opposing_pieces, U64 pin_path, struct mqueue* q);


void knight_moves(const U8 sqr, const U64 own_pieces, U64 constraint, struct mqueue* q);


void king_moves(const U8 sqr, const U64 own_pieces, const U64 attacks, struct mqueue* q);


void bishop_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path, struct mqueue* q);


void rook_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path, struct mqueue* q);


void queen_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path, struct mqueue* q);