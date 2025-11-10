#include "pos.h"

#include "format.h" // for calls inside from_fen()
#include "piece.h"
#include "data.h"

#ifdef TEST

#include "debug.h"

#endif 

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


#define U64_MAX        0xFFFFFFFFFFFFFFFFULL

#define SECOND_RANK    0x00FF000000000000ULL
#define SEVENTH_RANK   0x000000000000FF00ULL


U8 queue_count = 0;
U32 queue[255];

U8 piece_count[2];
struct piece pieces[2][16];

U64 side_masks[2];
U64 piece_masks[12] = {0ULL};
U64 board_mask;
U64 en_passant_sqr;

U8 castling_rights; 

enum colour side;

enum board_state pos_state;


U8 get_move_count() {
	return queue_count;
}

U32* get_move_list(){
	return queue;
}

void add_piece(enum piece_id id, U8 sqr, enum colour c) {
    
	struct piece p = {sqr, id};
	pieces[c][piece_count[c]++] = p;
	side_masks[c] |= (1ULL << sqr);
	board_mask |= (1ULL << sqr);
	piece_masks[id] |= 1ULL << sqr;
    
}


void swap_king(enum colour c) {
	struct piece temp = pieces[c][0];
	pieces[c][0] = pieces[c][piece_count[c] - 1];
	pieces[c][piece_count[c] - 1] = temp;
}


U8 from_fen(const char* fen) {

    // read the board 
    U8 index = 0;
    U8 sqr = 0; 

	board_mask = 0;
    while (fen[index] != ' ') {
	
		if (fen[index] == '/') {
			index++; 
			continue; 
		}

		enum piece_id id = id_from_char(fen[index]); // engio.h
		
		if (id != NONE) {
			
			// kings are always at index 0, hence the swaps after add_piece

			if (id == WHITE_KING) {
				if (pieces[WHITE][0].id == WHITE_KING) {
					printf("Invalid fen: more than 1 white king found. Exit.");
					return 1; 
				}
				add_piece(WHITE_KING, sqr, WHITE);
				swap_king(WHITE);
			}
			else if (id == BLACK_KING) {
				if (pieces[BLACK][0].id == BLACK_KING) {
					printf("Invalid fen: more than 1 black king found. Exit.");
					return 1; 
				}
				add_piece(BLACK_KING, sqr, BLACK);	
				swap_king(BLACK);
			}
			else add_piece(id, sqr, id < 6 ? WHITE : BLACK);
					
			sqr++; 
			index++;
			continue; 
		}
		
		// else, we are left with the number of empty squares 
		U8 empty = fen[index] - '0';
		for (; empty > 0; empty--) sqr++; 
		index++; 
    }

    if (pieces[WHITE][0].id != WHITE_KING || pieces[BLACK][0].id != BLACK_KING) {
		printf("Invalid fen: one or both kings missing. Exit");
		return 1;
    } 
  
    // read the rest of the fen data 
    index++; U8 part = 0; U8 part_start = index;
    
    // (ignoring plies and fifty rule for now)
    
    while (part < 3) {
      
		if (fen[index] != ' ') {
			index++;
			continue; 
		}
		
		switch (part) {
			
		case 0:
			if (fen[part_start] == 'w') side = WHITE; 
			else side = BLACK; 
			break; 
			
		case 1:
			if (fen[part_start] == '-') castling_rights = 0; 
			else castling_rights = castle_from_fen(fen, part_start, index);
			break; 
		
		case 2:
			if (fen[part_start] == '-') en_passant_sqr = 0ULL; 
			else en_passant_sqr = 1ULL << sqr_from_uci(&fen[part_start]); 
			break; 
		} 
		
		part++; index++; part_start = index;  
    }

	return 0;

}


// U8 get_dir_index(U8 s, U8 t) {

// 	int dx_raw = (t & 7) - (s & 7);
//     int dy_raw = (t >> 3) - (s >> 3);

//     int dx = (dx_raw > 0) - (dx_raw < 0); 
//     int dy = (dy_raw > 0) - (dy_raw < 0); 

// 	return (U8)((dx + 1) + ((dy + 1) * 3));      

// }

// U64 isolate_pin_path(U8 sqr, U8 piece_sqr, U64 pin_paths) {

// 	U64 piece_mask = 1ULL << piece_sqr;
// 	return pin_paths & king_rays[sqr][get_dir_index(sqr, piece_sqr)] & ~piece_mask & mtz(piece_mask);
// }


void enqueue(const U8 sqr, U64 moves, enum move_type mtype) {
    
    while (moves) {	
		U8 destination_sqr = __builtin_ctzll(moves);
		queue[queue_count++] = get_move(sqr, destination_sqr, mtype, 0);
		moves &= moves - 1; 
    }
}


U32* get_moves() {
    
    const enum colour wait 			= side ^ 1;
    const U8 moving_king_sqr 		= pieces[side][0].sqr;
	const U64 moving_king_mask 		= 1ULL << moving_king_sqr; // dependent on above
    const U64 waiting_side_mask 	= side_masks[wait];
    const U64 moving_side_mask 		= side_masks[side];

    ///////////////////////////////////////////////// WAITING SIDE 

	U64 attacks 				= 0;
	U64 check_path 				= 0;
	U64 checkers				= 0;
	// for slider king x-ray
	U64 board_mask_no_king 		= board_mask & ~moving_king_mask; 
	const U64 king_rook_att 	= rook_att(moving_king_sqr, board_mask);
	const U64 king_bishop_att 	= bishop_att(moving_king_sqr, board_mask);

	for (U8 i = 0; i < piece_count[wait]; i++) {

		struct piece p = pieces[wait][i];
		U64 att = 0;
		U64 sqr_mask = 1ULL << p.sqr;

		switch(p.id) {
			
			case WHITE_PAWN: 
				att |= wpawn_att(p.sqr);
				check_path |= sqr_mask & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				attacks |= att;
				break;
			case BLACK_PAWN:
				att |= bpawn_att(p.sqr);
				check_path |= sqr_mask & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				attacks |= att;
				break;
			case WHITE_KING:
			case BLACK_KING:
				attacks |= king_att(p.sqr);
				break;
			case WHITE_KNIGHT:
			case BLACK_KNIGHT:
				att = knight_att(p.sqr);
				check_path |= sqr_mask & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				attacks |= att;
				break;
			case WHITE_BISHOP:
			case BLACK_BISHOP: 
				att = bishop_att(p.sqr, board_mask_no_king);
				// this generates the attacks again because the king is x-rayed in the above, 
				// otherwise the intersection below would add squares behind the king to the check_path
				check_path |= (((king_bishop_att & bishop_att(p.sqr, board_mask)) & ~moving_king_mask) | sqr_mask) & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				attacks |= att;
				break;
			case WHITE_ROOK:
			case BLACK_ROOK: 
				att = rook_att(p.sqr, board_mask_no_king);
				// this generates the attacks again because the king is x-rayed in the above, 
				// so the intersection below would add squares behind the king to the check_path
				check_path |= (((king_rook_att & rook_att(p.sqr, board_mask)) & ~moving_king_mask) | sqr_mask) & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				attacks |= att;
				break;
			case WHITE_QUEEN:
			case BLACK_QUEEN: {

				U64 attb = bishop_att(p.sqr, board_mask_no_king);
				check_path |= (((king_bishop_att & bishop_att(p.sqr, board_mask)) & ~moving_king_mask) | sqr_mask) & mtz(moving_king_mask & attb);
				checkers |= sqr_mask & mtz(moving_king_mask & attb);

				U64 attr = rook_att(p.sqr, board_mask_no_king);
				check_path |= (((king_rook_att & rook_att(p.sqr, board_mask)) & ~moving_king_mask) | sqr_mask) & mtz(moving_king_mask & attr);
				checkers |= sqr_mask & mtz(moving_king_mask & attr);

				attacks |= (attb | attr);
				break;
			}
			default:
				break;

		}
	}


    ///////////////////////////////////////////////// MOVING SIDE 
 
	king_moves(pieces[side][0].sqr, moving_side_mask, attacks);
	enqueue(
		moving_king_sqr, 
		castle(moving_king_sqr, castling_rights, board_mask, side, attacks, check_path),
		CASTLE
		);

	U8 moving_pieces_count = piece_count[side] * (__builtin_popcountll(checkers) < 2);

	for (U8 i = 1; i < moving_pieces_count; i++) {

		struct piece p = pieces[side][i];

		switch (p.id) {

			case WHITE_PAWN:
				enqueue(p.sqr,
					wpawn_moves(
					p.sqr, 
					en_passant_sqr, 
					board_mask, 
					waiting_side_mask, 
					mto(check_path)
				),
				(1ULL << p.sqr) & SEVENTH_RANK
				);
				break;
			case BLACK_PAWN:
				enqueue(p.sqr,
					bpawn_moves(
					p.sqr, 
					en_passant_sqr, 
					board_mask, 
					waiting_side_mask, 
					mto(check_path)
					),
					(1ULL << p.sqr) & SECOND_RANK
				);
				break;
			case WHITE_KNIGHT:
			case BLACK_KNIGHT:
				enqueue(p.sqr,
					knight_moves(
					p.sqr, 
					moving_side_mask, 
					mto(check_path)
					),
					NORMAL
				);
				break;
			case WHITE_BISHOP:
			case BLACK_BISHOP:
				enqueue(p.sqr,
					bishop_moves(
					p.sqr, 
					board_mask, 
					moving_side_mask, 
					// a knight can never move if pinned 
					mto(check_path)
					),
					NORMAL
				);
				break;
			case WHITE_ROOK:
			case BLACK_ROOK:
				enqueue(p.sqr,
					rook_moves(
					p.sqr, 
					board_mask, 
					moving_side_mask, 
					mto(check_path)
					),
					NORMAL
				);
				break;
			case WHITE_QUEEN:
			case BLACK_QUEEN:
				enqueue(p.sqr,
					queen_moves(
					p.sqr, 
					board_mask, 
					moving_side_mask, 
					mto(check_path)
					),
					NORMAL
				);
				break;
			default:
				break;
		}
	}

	U8  check = !!check_path;
	U8 	move_count = !!queue_count;

	pos_state = (enum board_state)((check & move_count) + ((check & !move_count) << 1) + ((!check & !move_count) * 3));

	return queue;
    
}





