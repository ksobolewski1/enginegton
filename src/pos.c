#include "pos.h"

#include "format.h" // for calls inside from_fen()
#include "piece.h"
#include "mqueue.h"
#include "data.h"
#include "move.h"

#ifdef TEST

#include "debug.h"

#endif 

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


#define U64_MAX        0xFFFFFFFFFFFFFFFFULL


U8 get_dir_index(U8 s, U8 t) {

	int dx_raw = (t & 7) - (s & 7);
    int dy_raw = (t >> 3) - (s >> 3);

    int dx = (dx_raw > 0) - (dx_raw < 0); 
    int dy = (dy_raw > 0) - (dy_raw < 0); 

	return (U8)((dx + 1) + ((dy + 1) * 3));      

}


U64 isolate_pin_path(U8 sqr, U8 piece_sqr, U64 pin_paths) {

	U64 piece_mask = 1ULL << piece_sqr;
	return pin_paths & king_rays[sqr][get_dir_index(sqr, piece_sqr)] & ~piece_mask & mtz(piece_mask);
}


void free_pos(struct position* pos) {

    free_piece(pos->pawns[WHITE]); free_piece(pos->pawns[BLACK]);
    free_piece(pos->pieces[WHITE]); free_piece(pos->pieces[BLACK]);
    free(pos);
    
}


void add_piece(struct position* pos, struct piece* piece, struct piece* head, U8 sqr) {
    
    pos->board[sqr] = piece;
    
    if (head->next == NULL) {
		head->next = head->prev = piece;
		piece->prev = head;
		return; 
    }
    
    head->prev->next = piece;
    piece->prev = head->prev;
    head->prev = piece;
}


struct piece* remove_piece(struct position* pos, U8 sqr) {

    struct piece* p = pos->board[sqr];
    pos->board[sqr] = NULL;

    // cut the piece from the linked lists

    // the side mask would need to be updated as well

    return p;
}


struct position* from_fen(const char* fen) {

    struct position* pos = (struct position*)malloc(sizeof(struct position));
    if (pos == NULL) {
		fprintf(stderr, "Error: Failed to allocate memory for struct 'position'\n");
		return NULL; 
    }

    // read the board 
    U8 index = 0;
    U8 sqr = 0; 
    while (fen[index] != ' ') {
	
		if (fen[index] == '/') {
			index++; 
			continue; 
		}

		enum piece_id id = id_from_char(fen[index]); // engio.h
		
		// adding kings first
		if (id != NONE) {

			struct piece* p = get_piece(id, sqr);
			pos->board[sqr] = p;
			
			if (id == WHITE_KING) {
			if (pos->pieces[WHITE] != NULL) {
				printf("Invalid fen: more than 1 white king found. Exit.");
				return NULL; 
			}
			pos->pieces[WHITE] = p;
			}
			else if (id == BLACK_KING) {
			if (pos->pieces[BLACK] != NULL) {
				printf("Invalid fen: more than 1 black king found. Exit.");
				return NULL; 
			}
			pos->pieces[BLACK] = p;
			}
					
			sqr++; 
			index++;
			continue; 
		}
		
		// else, we are left with the number of empty squares 
		U8 empty = fen[index] - '0';
		for (; empty > 0; empty--) {
			pos->board[sqr] = NULL;
			sqr++; 
		}
		
		index++; 
    }

    if (pos->pieces[WHITE] == NULL || pos->pieces[BLACK] == NULL) {
	printf("Invalid fen: one or both kings missing. Exit");
	return NULL; 
    } 

    pos->board_mask = 0; 
    // kings were placed as heads of pos->pieces' linked lists; adding the rest of the pieces 
    for (int i = 0; i < 64; i++) {
	
		if (pos->board[i] == NULL) continue;
		
		pos->board_mask |= (1ULL << i);
		
		enum piece_id id = pos->board[i]->id;
		enum colour piece_colour = id < 6 ? WHITE : BLACK;
		U8 colour_offset = piece_colour == WHITE ? 0 : 6;
		pos->side_masks[piece_colour] |= (1ULL << i);

		if (id == WHITE_KING || id == BLACK_KING) continue;

		if (id - colour_offset == 1) {
			if (pos->pawns[piece_colour] == NULL) pos->pawns[piece_colour] = pos->board[i];
			else add_piece(pos, pos->board[i], pos->pawns[piece_colour], i);
		}
		else {
			if (pos->pieces[piece_colour] == NULL) pos->pieces[piece_colour] = pos->board[i];
			else add_piece(pos, pos->board[i], pos->pieces[piece_colour], i);
		}

    }
  
    // read the rest of the fen data 
    index++; U8 part = 0; U8 part_start = index;
    
    // ignoring plies and fifty for now
    
    while (part < 3) {
      
		if (fen[index] != ' ') {
			index++;
			continue; 
		}
		
		switch (part) {
			
		case 0:
			if (fen[part_start] == 'w') pos->side = WHITE; 
			else pos->side = BLACK; 
			break; 
			
		case 1:
			if (fen[part_start] == '-') pos->castling_rights = 0; 
			else pos->castling_rights = castle_from_fen(fen, part_start, index);
			break; 
		
		case 2:
			if (fen[part_start] == '-') pos->en_passant_sqr = 0ULL; 
			else pos->en_passant_sqr = 1ULL << sqr_from_uci(&fen[part_start]); 
			break; 
		} 
		
		part++; index++; part_start = index;  
    }

    return pos; 
}


char* to_fen(struct position* pos) {

    return NULL; 
}


// MAIN FUNCTIONS


enum board_state get_moves(struct position* pos, struct mqueue* queue) {
    
    const enum colour turn 			= pos->side;
    const enum colour wait 			= pos->side ^ 1;

    const U8 moving_king_sqr 		= pos->pieces[turn]->sqr;
	const U64 moving_king_mask 		= 1ULL << moving_king_sqr; // dependent on above (?)

    const U64 waiting_side_mask 	= pos->side_masks[wait];
    const U64 moving_side_mask 		= pos->side_masks[turn];
    const U64 board_mask 			= pos->board_mask;

    ///////////////////////////////////////////////// WAITING SIDE 

	U64 attacks 				= 0;
	U64 check_path 				= 0;
	U64 checkers				= 0;
	// for slider king x-ray
	U64 board_mask_no_king 		= board_mask & ~moving_king_mask; 
	// pin detection masks
	const U64 king_rook_att 	= rook_att(moving_king_sqr, board_mask);
	const U64 king_bishop_att 	= bishop_att(moving_king_sqr, board_mask);
	U64 pin_paths 				= 0;

	struct piece* current_piece = pos->pawns[wait];

	while (current_piece != NULL) {

		U64 att = 0;
		U8 sqr = current_piece->sqr;
		U64 sqr_mask = 1ULL << sqr;

		switch(current_piece->id) {
			
			case WHITE_PAWN: 
				att |= wpawn_att(sqr);
				check_path = sqr_mask & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				attacks |= att;
				break;
			case BLACK_PAWN:
				att |= bpawn_att(sqr);
				check_path = sqr_mask & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				attacks |= att;
				break;
			default:
				break;

		}

		current_piece = current_piece->next;
	}

	current_piece 			= pos->pieces[wait];

	while (current_piece != NULL) {

		U64 att = 0;
		U8 sqr = current_piece->sqr;
		U64 sqr_mask = 1ULL << sqr;
		
		switch(current_piece->id) {
			
			case WHITE_KING:
			case BLACK_KING:
				attacks |= king_att(sqr);
				break;
			case WHITE_KNIGHT:
			case BLACK_KNIGHT:
				att = knight_att(sqr);
				check_path = sqr_mask & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				attacks |= att;
				break;
			case WHITE_BISHOP:
			case BLACK_BISHOP: 
				att = bishop_att(sqr, board_mask_no_king);
				// this generates the attacks again because the king is x-rayed in the above, 
				// otherwise the intersection below would add squares behind the king to the check_path
				check_path = ((king_bishop_att & bishop_att(sqr, board_mask)) & ~moving_king_mask) & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				pin_paths |= ((king_bishop_att | att) & king_rays[sqr][get_dir_index(moving_king_sqr, sqr)]) & mtz(king_bishop_att & att) & (1ULL << sqr);
				attacks |= att;
				break;
			case WHITE_ROOK:
			case BLACK_ROOK: 
				att = rook_att(sqr, board_mask_no_king);
				// this generates the attacks again because the king is x-rayed in the above, 
				// so the intersection below would add squares behind the king to the check_path
				check_path = ((king_rook_att & rook_att(sqr, board_mask)) & ~moving_king_mask) & mtz(moving_king_mask & att);
				checkers |= sqr_mask & mtz(moving_king_mask & att);
				pin_paths |= ((king_rook_att | att) & king_rays[sqr][get_dir_index(moving_king_sqr, sqr)]) & mtz(king_bishop_att & att) & (1ULL << sqr);
				attacks |= att;
				break;
			case WHITE_QUEEN:
			case BLACK_QUEEN: {
				U64 attb = bishop_att(sqr, board_mask_no_king);
				check_path = ((king_bishop_att & bishop_att(sqr, board_mask)) & ~moving_king_mask) & mtz(moving_king_mask & attb);
				checkers |= sqr_mask & mtz(moving_king_mask & attb);
				pin_paths |= ((king_bishop_att | attb) & king_rays[sqr][get_dir_index(moving_king_sqr, sqr)]) & mtz(king_bishop_att & attb) & (1ULL << sqr);
				U64 attr = rook_att(sqr, board_mask_no_king);
				check_path = ((king_rook_att & rook_att(sqr, board_mask)) & ~moving_king_mask) & mtz(moving_king_mask & attr);
				checkers |= sqr_mask & mtz(moving_king_mask & attr);
				pin_paths |= ((king_rook_att | attr) & king_rays[sqr][get_dir_index(moving_king_sqr, sqr)]) & mtz(king_rook_att & attr) & (1ULL << sqr);
				attacks |= (attb | attr);
				break;
			}
			default:
				break;
		}

		current_piece = current_piece->next;
	}


    ///////////////////////////////////////////////// MOVING SIDE 

	current_piece = pos->pieces[turn]; 
	king_moves(current_piece->sqr, moving_side_mask, attacks, queue);

	// DOUBLE CHECK HERE

	castle(moving_king_sqr, pos->castling_rights, board_mask, turn, attacks, check_path, queue);

	current_piece = current_piece->next;
	while (current_piece != NULL && __builtin_popcount(checkers) < 2) {

		U8 sqr = current_piece->sqr;

		switch (current_piece->id) {

			case WHITE_KNIGHT:
			case BLACK_KNIGHT:
				knight_moves(
					sqr, 
					moving_side_mask, 
					mto(mto(isolate_pin_path(moving_king_sqr, sqr, pin_paths)) & check_path), 
					queue
				);
				break;
			case WHITE_BISHOP:
			case BLACK_BISHOP:
				bishop_moves(
					sqr, 
					board_mask, 
					moving_side_mask, 
					mto(mto(isolate_pin_path(moving_king_sqr, sqr, pin_paths)) & check_path), 
					queue
				);
				break;
			case WHITE_ROOK:
			case BLACK_ROOK:
				rook_moves(
					sqr, 
					board_mask, 
					moving_side_mask, 
					mto(mto(isolate_pin_path(moving_king_sqr, sqr, pin_paths)) & check_path), 
					queue
				);
				break;
			case WHITE_QUEEN:
			case BLACK_QUEEN:
				queen_moves(
					sqr, 
					board_mask, 
					moving_side_mask, 
					mto(mto(isolate_pin_path(moving_king_sqr, sqr, pin_paths)) & check_path), 
					queue
				);
				break;
			default:
				break;
		}

		current_piece = current_piece->next;

	}

	current_piece = pos->pawns[turn];
	const U64 en_passant_mask = pos->en_passant_sqr;

	while (current_piece != NULL && __builtin_popcount(checkers) < 2) {

		U8 sqr = current_piece->sqr;

		switch (current_piece->id) {

			case WHITE_PAWN:
				wpawn_moves(
					sqr, 
					en_passant_mask, 
					board_mask, 
					waiting_side_mask, 
					mto(mto(isolate_pin_path(moving_king_sqr, sqr, pin_paths)) & check_path),  
					queue
				);
				break;
			case BLACK_PAWN:
				bpawn_moves(
					sqr, 
					en_passant_mask, 
					board_mask, 
					waiting_side_mask, 
					mto(mto(isolate_pin_path(moving_king_sqr, sqr, pin_paths)) & check_path), 
					queue
				);
				break;
			default:
				break;
		}

		current_piece = current_piece->next;
	}

	U8  check = !!check_path;
	U8 	move_count = !!queue->N;

	return (enum board_state)((check & move_count) + ((check & !move_count) << 1) + ((!check & !move_count) * 3));
    
}


// move-making function 
enum board_state update_pos(struct position* pos, struct mqueue* queue, U32 move) {

    enum board_state state = QUIET;

	// update piece count

    return state;
}



