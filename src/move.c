#include "move.h"


#include "mqueue.h"


U64 wpawn_att(const U8 sqr);
U64 bpawn_att(const U8 sqr);
U64 knight_att(const U8 sqr);
U64 king_att(const U8 sqr);


#define FOURTH_RANK    0x000000FF00000000ULL
#define FIFTH_RANK     0x00000000FF000000ULL


// to use the x86 bsr instruction 
#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))


// rating is 0 for now 
// MUST PASS IN THE MOVE TYPE
void from_bit_scan_enqueue(const U8 sqr, const U64 moves, struct mqueue* queue) {
    
    I64 signed_moves = (I64)moves; 
    while (signed_moves != 0) {	
		I64 lsb = signed_moves & -signed_moves;
		U8 loglsb = (U8)LOG2((U64)lsb);
		enqueue(queue, get_move(sqr, loglsb, NORMAL, 0));
		signed_moves ^= lsb;
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


void wpawn_moves(const U8 sqr, const U64 en_passant_sqr, U64 blockers, const U64 opposing_pieces, U64 pin_path, struct mqueue* q) {

    U64 moves = (1ULL << (sqr - 8)) & ~blockers;
    moves |= ((moves >> 8) & ~blockers) & FOURTH_RANK;
    moves |= wpawn_attacks[sqr] & opposing_pieces;
    moves |= en_passant_sqr;

    from_bit_scan_enqueue(sqr, moves, q);

    // PROMOTION
}


void bpawn_moves(const U8 sqr, const U64 en_passant_sqr, U64 blockers, const U64 opposing_pieces, U64 pin_path, struct mqueue* q) {

    U64 moves = (1ULL << (sqr + 8)) & ~blockers;
    moves |= ((moves << 8) & ~blockers) & FIFTH_RANK; 
    moves |= bpawn_attacks[sqr] & opposing_pieces;
    moves |= en_passant_sqr;

    from_bit_scan_enqueue(sqr, moves, q);

    // PROMOTION

}

// ENQUEUE SPECIAL MOVES DIRECTLY


void knight_moves(const U8 sqr, const U64 own_pieces, struct mqueue* q) {

    from_bit_scan_enqueue(sqr, knight_att(sqr) & ~own_pieces, q);
}


void king_moves(const U8 sqr, const U64 own_pieces, const U64 attacks, struct mqueue* q) {

    from_bit_scan_enqueue(sqr, king_att(sqr) & ~(own_pieces | attacks), q);
}


void bishop_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path, struct mqueue* q) {

    U64 moves = bishop_att(sqr, board_mask) & ~own_pieces;
    from_bit_scan_enqueue(sqr, moves, q);
}


void rook_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path, struct mqueue* q) {

    U64 moves = rook_att(sqr, board_mask) & ~own_pieces;
    from_bit_scan_enqueue(sqr, moves, q);
}


void queen_moves(const U8 sqr, const U64 board_mask, const U64 own_pieces, U64 pin_path, struct mqueue* q){
    
    U64 moves = (rook_att(sqr, board_mask) | bishop_att(sqr, board_mask)) & ~own_pieces;
    from_bit_scan_enqueue(sqr, moves, q);
}
