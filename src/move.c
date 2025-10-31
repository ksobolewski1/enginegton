#include "move.h"


#include "mqueue.h"


U64 wpawn_att(const U8 sqr);
U64 bpawn_att(const U8 sqr);
U64 knight_att(const U8 sqr);
U64 king_att(const U8 sqr);


#define FIRST_RANK     0xFF00000000000000ULL
#define FOURTH_RANK    0x000000FF00000000ULL
#define FIFTH_RANK     0x00000000FF000000ULL
#define EIGHT_RANK     0x00000000000000FFULL


void enqueue_moves(const U8 sqr, U64 moves, enum move_type mtype, struct mqueue* queue) {
    
    while (moves) {	
		U8 destination_sqr = __builtin_ctzll(moves);
		enqueue(queue, get_move(sqr, destination_sqr, mtype, 0));
		moves &= moves - 1; // ctzll called on 0 is undefined behaviour
    }
}


U64 bishop_att(const U8 sqr, const U64 blockers_mask) {
    
    U64 blockers = blockers_mask & bishop_masks[sqr];
    U8 k = (blockers * bishop_magics[sqr]) >> (64 - bishop_shifts[sqr]);
    return bishop_configs[sqr][k];
}


U64 rook_att(const U8 sqr, const U64 blockers_mask) {

    U64 blockers = blockers_mask & rook_masks[sqr];
    U8 k = (blockers * rook_magics[sqr]) >> (64 - rook_shifts[sqr]);
    return rook_configs[sqr][k];    
}


void wpawn_moves(const U8 sqr, const U64 en_passant_sqr, U64 blockers, const U64 opposing_pieces, U64 constraint, struct mqueue* q) {

    U64 moves = (1ULL << (sqr - 8)) & ~blockers;
    moves |= ((moves >> 8) & ~blockers) & FOURTH_RANK;
    moves |= wpawn_attacks[sqr] & opposing_pieces;
    moves |= en_passant_sqr;
    moves &= constraint;

    enqueue_moves(sqr, moves, moves & EIGHT_RANK, q);
}


void bpawn_moves(const U8 sqr, const U64 en_passant_sqr, U64 blockers, const U64 opposing_pieces, U64 constraint, struct mqueue* q) {

    U64 moves = (1ULL << (sqr + 8)) & ~blockers;
    moves |= ((moves << 8) & ~blockers) & FIFTH_RANK; 
    moves |= bpawn_attacks[sqr] & opposing_pieces;
    moves |= en_passant_sqr;
    moves &= constraint;

    enqueue_moves(sqr, moves, moves & FIRST_RANK, q);
}


void knight_moves(const U8 sqr, const U64 own_pieces, U64 constraint, struct mqueue* q) {

    enqueue_moves(sqr, (knight_att(sqr) & ~own_pieces) & constraint, NORMAL, q);
}


void king_moves(const U8 sqr, const U64 own_pieces, const U64 attacks, struct mqueue* q) {

    enqueue_moves(sqr, king_att(sqr) & ~(own_pieces | attacks), NORMAL, q);
}


void bishop_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 constraint, struct mqueue* q) {

    U64 moves = bishop_att(sqr, board_mask) & ~own_pieces;
    moves &= constraint;
    enqueue_moves(sqr, moves, NORMAL, q);
}


void rook_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 constraint, struct mqueue* q) {

    U64 moves = rook_att(sqr, board_mask) & ~own_pieces;
    moves &= constraint;
    enqueue_moves(sqr, moves, NORMAL, q);
}


void queen_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 constraint, struct mqueue* q){
    
    U64 moves = (rook_att(sqr, board_mask) | bishop_att(sqr, board_mask)) & ~own_pieces;
    moves &= constraint;
    enqueue_moves(sqr, moves, NORMAL, q);
}
