#include "moves.h"

#include "pos.h"
#include "mqueue.h"
#include "piece.h"
#include "data.h"

#ifdef TEST

#include "debug.h"

#endif 

#include <stdio.h>


/* todo
   hopper switch blocks, and pawns simplification
*/


// to use the x86 bsr instruction 
#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))


// rating is 0 for now 
void bit_scan(const U8 sqr, const U64 moves, struct mqueue* queue) {
    
    I64 signed_moves = (I64)moves; 
    while (signed_moves != 0) {	
	I64 lsb = signed_moves & ~signed_moves;
	// these logs can also be precomputed and put inside a hash map 
	U8 loglsb = (U8)LOG2((U64)lsb);
	enqueue(queue, get_move(sqr, loglsb, NORMAL, 0));
	signed_moves ^= lsb;
    }
}

// rating is 0 for now 
void promotion_bit_scan(const U8 sqr, const U64 moves, struct mqueue* queue) {
    I64 signed_moves = (I64)moves; 
    while (signed_moves != 0) {	
	I64 lsb = signed_moves & ~signed_moves;
	U8 loglsb = LOG2((U64)lsb);
	for (int i = 0; i < 4; i++) enqueue(queue, get_move(sqr, loglsb, i, 0));
	signed_moves ^= lsb;
    }
}


// L/SCM: long/short castle mask (all squares between the king and the rook)
// LSM: long sub-mask (the squares that the king traverses en route to, and including, the destination square)
#define WHITE_LCM 0x0E000000
#define WHITE_SCM 0x60000000
#define WHITE_LSM 0x0C000000
#define BLACK_LCM 0x0000000E
#define BLACK_SCM 0x000000C0
#define BLACK_LSM 0x0000000C


void white_castle(U8 castle_rights, U64 attacks, U64 block, struct mqueue* queue) {
    
    if ((castle_rights & 2) > 0	 
	&& ((WHITE_LCM & block) == 0 && (WHITE_LSM & attacks) == 0)) {
	enqueue(queue, get_move(60, 58, LONG_CASTLE, 0));
	}
    if ((castle_rights & 1) > 0
	&& ((WHITE_SCM & block) == 0 && (WHITE_SCM & attacks) == 0)) {
	enqueue(queue, get_move(60, 62, SHORT_CASTLE, 0));
    }
}

void black_castle(U8 castle_rights, U64 attacks, U64 block, struct mqueue* queue) {

    if ((castle_rights & 8) > 0
	&& ((BLACK_LCM & block) == 0 && (BLACK_LSM & attacks) == 0)) {
	enqueue(queue, get_move(4, 2, LONG_CASTLE, 0));
	}
    if ((castle_rights & 4) > 0 && ((BLACK_SCM & block) == 0 && (BLACK_SCM & attacks) == 0)) {
	enqueue(queue, get_move(4, 6, SHORT_CASTLE, 0));
    }
}

void (*castle_functions[2])(U8 castle_rights, U64 attacks, U64 block, struct mqueue* queue) =
{
    black_castle, white_castle 
};


U64 bishop_att(const U8 sqr, const U64 blockers_mask) {
    
    U64 blockers = blockers_mask & bishop_masks[sqr];
    U8 k = (blockers * bishop_magics[sqr]) >> (64 - bishop_shifts[sqr]);
    U64 attack = bishop_configs[sqr][k];
    
    return attack;
}

U64 rook_att(const U8 sqr, const U64 blockers_mask) {

    U64 blockers = blockers_mask & rook_masks[sqr];
    U8 k = (blockers * rook_magics[sqr]) >> (64 - rook_shifts[sqr]);
    U64 attack = rook_configs[sqr][k];    
    
    return attack; 
}


U8 checks_and_pins(const U8 sqr, const U64 att, const U64 moving_king_mask, const U64 king_rays,
		   const U64 board_mask, U64(*att_func)(const U8, const U64), U64* check_path, U64* pin_map,
		   U64* pin_paths, U8* state) {

    if ((att & moving_king_mask) > 0) {
	(*state)++;
	*check_path = (king_rays & att) | (1ULL << sqr);
	return 1; 
    }
	    
    U64 intersect = king_rays & att;
    if (intersect > 0) {
	U8 pinned_sqr = (U8)LOG2(intersect);
	*pin_map |= intersect;
	pin_paths[pinned_sqr] = king_rays & att & att_func(pinned_sqr, board_mask) & ~moving_king_mask;
	return 1;
    }
    return 0;
}



U64 switch_sliders(const U8 sqr, const enum piece_id id, const U64 board_mask, const U64 own_side_mask) {

    U64 moves = 0;
    
    switch (id) {
	
    case WHITE_BISHOP:						
    case BLACK_BISHOP: 					
	moves = bishop_att(sqr, board_mask) & ~own_side_mask;		
	break;
	
    case WHITE_ROOK:						
    case BLACK_ROOK: 
	moves = rook_att(sqr, board_mask) & ~own_side_mask;		
	break;
	
    case WHITE_QUEEN:						
    case BLACK_QUEEN:
	moves = (bishop_att(sqr, board_mask) | rook_att(sqr, board_mask)) & ~own_side_mask;
	break;

    default:
	break;
    }
    
    return moves; 
}

enum board_state get_moves(struct position* pos, struct mqueue* queue) {
    
    const enum colour turn = pos->side;
    const enum colour wait = turn ^ 1;
    U8 state = 0; // cast to enum board_state upon return  

    U64 check_path = 0;  
    U64 pin_map = 0;
    const U64* pin_paths = pos->pin_paths;
    
    struct piece* moving = pos->hoppers[turn];
    struct piece* waiting = pos->hoppers[wait];

    const U64 own_side_mask = pos->side_masks[turn];
    const U64 opp_side_mask = pos->side_masks[turn ^ 1];
    const U64 board_mask = pos->board_mask;
    const U64 en_passant_mask = 1ULL << pos->en_passant_sqr;

    const U8 moving_king_sqr = moving->sqr; 
    const U64 moving_king_mask = 1ULL << moving_king_sqr;

    U64 attacks = king_attacks[waiting->sqr];

    ///////////////////////////////////////////////// WAITING SIDE 
    // iterate over the hoppers to get attacks (king handled above)
    waiting = waiting->next;    
    while (waiting != NULL) {

	U8 sqr = waiting->sqr;
	enum piece_id id = waiting->id;
	U64 att = 0;
	
	switch (id) {
	    
	case WHITE_KNIGHT:
	case BLACK_KNIGHT:
	    att = knight_attacks[sqr];
	    break;

	case WHITE_PAWN:
	    att = wpawn_attacks[sqr];
	    break;

	case BLACK_PAWN:
	    att = bpawn_attacks[sqr];
	    break;

	default:
	    break;
	}
	
	attacks |= att; 

	if ((att & moving_king_mask) > 0) {
	    state++;
	    check_path |= (1ULL << sqr);
	}
	waiting = waiting->next;
    }
    
    // iterate over the sliders to get attacks
    waiting = pos->sliders[wait];
    // get king rays (bishop and rook moves from king's perspective),
    // used to detect pins, and to obtain check and pin paths
    U64 king_bishop_rays = bishop_att(moving_king_sqr, board_mask); 
    U64 king_rook_rays = rook_att(moving_king_sqr, board_mask);
    while (waiting != NULL) {

	U8 sqr = waiting->sqr;
	U64 att = 0;
	
	switch (waiting->id) {

	case WHITE_BISHOP:
	case BLACK_BISHOP:
	    att = bishop_att(sqr, board_mask);
	    checks_and_pins(sqr, att, moving_king_mask, king_bishop_rays, board_mask, bishop_att,
			    &check_path, &pin_map, &pin_paths, &state);
	    break;

	case WHITE_ROOK:
	case BLACK_ROOK:
	    att = rook_att(sqr, board_mask);
	    checks_and_pins(sqr, att, moving_king_mask, king_rook_rays, board_mask, rook_att,
			    &check_path, &pin_map, &pin_paths, &state);
	    break;
	case WHITE_QUEEN:
	case BLACK_QUEEN: { 
	    U64 attb = bishop_att(sqr, board_mask);
	    if (checks_and_pins(sqr, attb, moving_king_mask, king_bishop_rays, board_mask, bishop_att,
				&check_path, &pin_map, &pin_paths, &state)) {
		att = attb;
		break;
	    }
	    // if the queen checks the king, or pins a piece, as a bishop, it cannot do either of those things
	    // as a rook
	    U64 attr = rook_att(sqr, board_mask);
	    checks_and_pins(sqr, attr, moving_king_mask, king_rook_rays, board_mask, rook_att,
			    &check_path, &pin_map, &pin_paths, &state);
	    att = attb | attr;
	    break;
	}
	default:
	    break;
	}
	
	attacks |= att;
	waiting = waiting->next;
    }

    
    ///////////////////////////////////////////////// MOVING SIDE 
    
    // get the moves for the king
    bit_scan(moving_king_sqr,
	(king_attacks[moving_king_sqr] & ~attacks) & ~own_side_mask, queue);

    // if quiet 
    if (state == 0) {

	(*castle_functions[turn])(pos->castling_rights, attacks, board_mask, queue); // castling 
	
	moving = moving->next; 
	while (moving != NULL) {

	    U8 sqr = moving->sqr;
	    U64 moves = 0;
  
	    switch (moving->id) {
	
	    case WHITE_KNIGHT:	 
	    case BLACK_KNIGHT:				
		moves = knight_attacks[sqr] & ~own_side_mask;       
		break;
	
	    case WHITE_PAWN: {

		U64 att = wpawn_attacks[sqr];
		moves = att & opp_side_mask;
		if (((1ULL << (sqr - 8)) & board_mask) == 0)
		    moves |= wpawn_pushes[sqr];
		
		if (sqr > 47) {
		    if (((1ULL << sqr) & pin_map) > 0) moves &= pin_paths[sqr];
		    promotion_bit_scan(sqr, moves, queue);
		    goto NEXT; 
		}
		    
		if ((att & en_passant_mask) > 0)
		    enqueue(queue, get_move(sqr, pos->en_passant_sqr, EN_PASSANT, 0));
		
		break;
	    }
	    case BLACK_PAWN: {
		
		U64 att = bpawn_attacks[sqr];
		moves = att & opp_side_mask; 
		if (((1ULL << (sqr + 8)) & board_mask) == 0)
		    moves |= bpawn_pushes[sqr];
		
		if (sqr < 16) {
		    if (((1ULL << sqr) & pin_map) > 0) moves &= pin_paths[sqr];
		    promotion_bit_scan(sqr, moves, queue);
		    goto NEXT; 
		}
		
		if ((att & en_passant_mask) > 0)
		    enqueue(queue, get_move(sqr, pos->en_passant_sqr, EN_PASSANT, 0));
		
		break;
	    }		
	    default:						
		break;					
	    }
	    
	    // check if the piece is pinned and update its move mask 
	    if (((1ULL << sqr) & pin_map) > 0) moves &= pin_paths[sqr]; 
	    bit_scan(sqr, moves, queue);

	NEXT:
	    moving = moving->next; 
	}

	// sliders 
	moving = pos->sliders[turn];
	while (moving != NULL) {

	    U8 sqr = moving->sqr;
	    U64 moves = switch_sliders(sqr, moving->id, board_mask, own_side_mask);
	   
	    if (((1ULL << sqr) & pin_map) > 0) moves &= pin_paths[sqr];
	    bit_scan(sqr, moves, queue);   
	    moving = moving->next; 
	}	
    }

    // if check 
    else if (state == 1) {

	moving = moving->next; 
	
	while (moving != NULL) {

	    U8 sqr = moving->sqr;
	    if (((1ULL << sqr) & pin_map) > 0) continue; 

	    U64 moves = 0;
	    
	    switch (moving->id) {
	
	    case WHITE_KNIGHT:	 
	    case BLACK_KNIGHT:				
		moves = knight_attacks[sqr] & ~own_side_mask;
		break;
	
	    case WHITE_PAWN: {
		
		U64 att = wpawn_attacks[sqr];
		moves = att & opp_side_mask; 
		if (((1ULL << (sqr - 8)) & board_mask) == 0)
		    moves |= wpawn_pushes[sqr];
		
		if (sqr > 47) {
		    moves &= check_path;
		    promotion_bit_scan(sqr, moves, queue);
		    goto NEXT_CHECK;
		}
		
		if ((att & en_passant_mask) > 0 && (check_path ^ (1ULL << (pos->en_passant_sqr - 8))) == 0)
		    enqueue(queue, get_move(sqr, pos->en_passant_sqr, EN_PASSANT, 0)); 

		break;
	    }
	    case BLACK_PAWN: {
		U64 att = bpawn_attacks[sqr];
		moves = att & opp_side_mask; 
		if (((1ULL << (sqr + 8)) & board_mask) == 0) moves |= bpawn_pushes[sqr];
		
		if (sqr < 16) {
		    moves &= check_path;
		    promotion_bit_scan(sqr, moves, queue);
		    goto NEXT_CHECK;
		}

		if ((att & en_passant_mask) > 0 && (check_path ^ (1ULL << (pos->en_passant_sqr + 8))) == 0)
		    enqueue(queue, get_move(sqr, pos->en_passant_sqr, EN_PASSANT, 0)); 

		break;
	    }
		
	    default:						
		break;					
	    }
	    
	    moves &= check_path;
	    bit_scan(sqr, moves, queue); 

	NEXT_CHECK:
	    moving = moving->next; 
	}

	moving = pos->sliders[turn];
	while (moving != NULL) {

	    U8 sqr = moving->sqr;
	    // if piece is pinned and there is a check, it cannot move 
	    if (((1ULL << sqr) & pin_map) > 0) continue; 

	    U64 moves = switch_sliders(sqr, moving->id, board_mask, own_side_mask) & check_path;
	    
	    bit_scan(sqr, moves, queue); 
	    moving = moving->next; 
	} 
    }

    // if double check, only the king can move, and these moves are already processed (if any)
    
    return (enum board_state)state; 
}


// move-making function 
enum board_state update_pos(struct position* pos, struct mqueue* queue, U32 move) {

    enum board_state state = QUIET;

    return state;
}



