#include "moves.h"

#include "pos.h"
#include "mqueue.h"
#include "piece.h"
#include "data.h"

#ifdef TEST

#include "debug.h"

#endif 

#include <stdio.h>
#include <math.h>

/* todo
   king x-ray 
*/


// to use the x86 bsr instruction 
#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))


// rating is 0 for now 
void bit_scan(const U8 sqr, const U64 moves, struct mqueue* queue) {

    printU64(moves);
    
    I64 signed_moves = (I64)moves; 
    while (signed_moves != 0) {	
	I64 lsb = signed_moves & -signed_moves;
	U8 loglsb = (U8)LOG2((U64)lsb);
	enqueue(queue, get_move(sqr, loglsb, NORMAL, 0));
	signed_moves ^= lsb;
    }
}

// rating is 0 for now 
void promotion_bit_scan(const U8 sqr, const U64 moves, struct mqueue* queue) {
    I64 signed_moves = (I64)moves; 
    while (signed_moves != 0) {	
	I64 lsb = signed_moves & -signed_moves;
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
    
    if ((castle_rights & 4) > 0
	&& ((BLACK_SCM & block) == 0 && (BLACK_SCM & attacks) == 0)) { 
	enqueue(queue, get_move(4, 6, SHORT_CASTLE, 0));
    }
}

void (*castle_functions[2])(U8 castle_rights, U64 attacks, U64 block, struct mqueue* queue) =
{
    black_castle, white_castle 
};


U8 en_passant_check(const U8 en_passant_sqr, const I8 dir,
		    const U64 check_path, const U64 att, const U64 en_passant_mask) {

    if (!check_path)
	return ((att & en_passant_mask) > 0);    
    return ((att & en_passant_mask) > 0 && (check_path ^ (1ULL << (en_passant_sqr + dir))) == 0);
    
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


U8 checks_and_pins(
    const U8 sqr,
    const U8 moving_king_sqr,
    U64* att,
    const U64 att_mask,
    const U64 moving_king_mask,
    const U64 king_rays,
    const U64 board_mask,
    U64(*att_func)(const U8, const U64),
    U64* check_path,
    U64* pin_map,
    U64* pin_paths,
    U8* state) {
    
    if ((*att & moving_king_mask) > 0) {
	(*state)++;
	*check_path = (king_rays & *att) | (1ULL << sqr);
	(*att) |= (king_attacks[moving_king_sqr] & att_mask); // add the square behind the king to attacks  
	return 1; 
    }
	    
    U64 intersect = king_rays & *att;
    if (intersect > 0) {
	U8 pinned_sqr = (U8)LOG2(intersect);
	*pin_map |= intersect;
	pin_paths[pinned_sqr] = king_rays & *att & att_func(pinned_sqr, board_mask) & ~moving_king_mask;
	return 1;
    }
    return 0;
}


U64 get_hop_attacks(struct piece* waiting, const U8 moving_king_sqr, U8* state,
		    const U64 moving_king_mask, U64* check_path) {

    U64 attacks = king_attacks[waiting->sqr];
    
    // iterate over the hoppers to get attacks (king handled above)
    
    waiting = waiting->next;    
    while (waiting != NULL) {

	U8 sqr = waiting->sqr;
	U64 att = 0;
	
	switch (waiting->id) {
	    
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
	waiting->attack_mask = att;
	
	if ((att & moving_king_mask) > 0) {
	    (*state)++;
	    (*check_path) |= (1ULL << sqr);
	}
	waiting = waiting->next;
    }

    return attacks; 
}


U64 get_slide_attacks(struct piece* waiting, const U8 moving_king_sqr, const U64 moving_king_mask,
		      const U64 board_mask, U8* state, U64* check_path, U64* pin_map, U64* pin_paths) {

    U64 attacks = 0;
    
    // iterate over the sliders to get attacks

    // get king rays (bishop and rook moves from king's perspective),
    U64 king_bishop_rays = bishop_att(moving_king_sqr, board_mask); 
    U64 king_rook_rays = rook_att(moving_king_sqr, board_mask);

    /* king rays method needs to be optimised					
       the rays should be obtained only if there is a possibility of check/pin
       and only once for a type 
    */   

    while (waiting != NULL) {

	U8 sqr = waiting->sqr;
	U64 att = 0;
	
	switch (waiting->id) {

	case WHITE_BISHOP:
	case BLACK_BISHOP:
	    
	    att = bishop_att(sqr, board_mask);
	    checks_and_pins(sqr, moving_king_sqr, &att, bishop_attacks[sqr],
			    moving_king_mask, king_bishop_rays, board_mask, bishop_att,
			    check_path, pin_map, pin_paths, state);
	    break;

	case WHITE_ROOK:
	case BLACK_ROOK:

	    att = rook_att(sqr, board_mask);
	    checks_and_pins(sqr, moving_king_sqr, &att, rook_attacks[sqr],
			    moving_king_mask, king_rook_rays, board_mask, rook_att,
			    check_path, pin_map, pin_paths, state);
	    break;
	    
	case WHITE_QUEEN:
	case BLACK_QUEEN: { 

	    U64 attb = bishop_att(sqr, board_mask);
	    if (checks_and_pins(sqr, moving_king_sqr, &attb, bishop_attacks[sqr],
				moving_king_mask, king_bishop_rays, board_mask, bishop_att,
				check_path, pin_map, pin_paths, state)) {
		att = attb | rook_att(sqr, board_mask);
		break;
	    }
	    
	    U64 attr = rook_att(sqr, board_mask);
	    checks_and_pins(sqr, moving_king_sqr, &attr, rook_attacks[sqr],
			    moving_king_mask, king_rook_rays, board_mask, rook_att,
			    check_path, pin_map, pin_paths, state);
	    att = attb | attr;
	    break;
	}
	    
	default:
	    break;
	}
	
	attacks |= att;
	waiting->attack_mask = att; 
	waiting = waiting->next;
    }

    return attacks; 
}

void get_hop_moves(enum colour turn, struct piece* moving, const U8 moving_king_sqr, const U8 castling_rights,
	      const U8 en_passant_sqr, const U64 attacks, const U64 own_side_mask, const U64 opp_side_mask,
	      const U64 check_path, const U64 pin_map, const U64* pin_paths, struct mqueue* queue) {

    U64 board_mask = own_side_mask & opp_side_mask;
    U64 en_passant_mask = 1ULL << en_passant_sqr;
   
    bit_scan(moving_king_sqr, (king_attacks[moving_king_sqr] & ~attacks) & ~own_side_mask, queue);
    
    if (!check_path)
	(*castle_functions[turn])(castling_rights, attacks, board_mask, queue); // castling  
    
    moving = moving->next; 
	
    while (moving != NULL) {

	U8 sqr = moving->sqr;
	if (check_path && ((1ULL << sqr) & pin_map) > 0) continue; 

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
		    
		if (check_path)
		    moves &= check_path;
		else if (((1ULL << sqr) & pin_map) > 0)
		    moves &= pin_paths[sqr];
		promotion_bit_scan(sqr, moves, queue);
		goto NEXT;
	    }

	    if (en_passant_check(en_passant_sqr, -8, check_path, att, en_passant_mask))
		enqueue(queue, get_move(sqr, en_passant_sqr, EN_PASSANT, 0)); 

	    break;
	}
		
	case BLACK_PAWN: {
	    U64 att = bpawn_attacks[sqr];
	    moves = att & opp_side_mask; 
	    if (((1ULL << (sqr + 8)) & board_mask) == 0) moves |= bpawn_pushes[sqr];
		
	    if (sqr < 16) {

		if (check_path)
		    moves &= check_path;
		else if (((1ULL << sqr) & pin_map) > 0)
		    moves &= pin_paths[sqr];
		promotion_bit_scan(sqr, moves, queue);
		goto NEXT;
	    }

	    if (en_passant_check(en_passant_sqr, 8, check_path, att, en_passant_mask)) 
		enqueue(queue, get_move(sqr, en_passant_sqr, EN_PASSANT, 0)); 

	    break;
	}
	    
	default:						
	    break;					
	}

	if (check_path) moves &= check_path;
	else if (((1ULL << sqr) & pin_map) > 0) moves &= pin_paths[sqr];

	bit_scan(sqr, moves, queue); 

    NEXT:
	moving->move_mask = moves; 
	moving = moving->next;
    }
}
	
void get_slide_moves(struct piece* moving, const U64 check_path, const U64 board_mask, const U64 pin_map,
		    const U64 own_side_mask, U64* pin_paths, struct mqueue* queue) {

    // +1 branch if either check or not, so need to look for ways to get rid of it 
    
    while (moving != NULL) {

	U8 sqr = moving->sqr;

	if (check_path && ((1ULL << sqr) & pin_map) > 0) continue;  
	
	U64 moves = 0;
   
	switch (moving->id) {
	
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

	// check_path instead if check
	if (check_path) moves &= check_path;
	else if (((1ULL << sqr) & pin_map) > 0) moves &= pin_paths[sqr];

	moving->move_mask = moves; 
	bit_scan(sqr, moves, queue);   
	moving = moving->next; 
    }
    
}



enum board_state get_moves(struct position* pos, struct mqueue* queue) {
    
    const enum colour turn = pos->side;
    const enum colour wait = turn ^ 1;

    U8 state = 0;

    U64 check_path = 0;  
    U64 pin_map = 0;

    const U8 moving_king_sqr = pos->hoppers[turn]->sqr;
    const U64 own_side_mask = pos->side_masks[turn];
    const U64 opp_side_mask = pos->side_masks[turn ^ 1];
    const U64 board_mask = pos->board_mask;
    U64* pin_paths = pos->pin_paths;

    ///////////////////////////////////////////////// WAITING SIDE 

    const U64 moving_king_mask = 1ULL << moving_king_sqr;

    U64 attacks = get_hop_attacks(pos->hoppers[wait], moving_king_sqr, &state, moving_king_mask, &check_path);
    
    attacks |= get_slide_attacks(pos->sliders[wait], moving_king_sqr, moving_king_mask, board_mask,
		      &state, &check_path, &pin_map, pin_paths);

    ///////////////////////////////////////////////// MOVING SIDE 

    if (state == 2) { // get the moves for the king
	bit_scan(moving_king_sqr,
		 (king_attacks[moving_king_sqr] & ~attacks) & ~own_side_mask, queue);
	goto END;
    }
    
    get_hop_moves(turn, pos->hoppers[turn], moving_king_sqr, pos->castling_rights, pos->en_passant_sqr,
		  attacks, own_side_mask, opp_side_mask, check_path, pin_map, pin_paths, queue);
	
    get_slide_moves(pos->sliders[turn], check_path, board_mask, pin_map,
		    own_side_mask, pin_paths, queue);
    
END:
    return (enum board_state)state;
    
}


// move-making function 
enum board_state update_pos(struct position* pos, struct mqueue* queue, U32 move) {

    enum board_state state = QUIET;

    return state;
}



