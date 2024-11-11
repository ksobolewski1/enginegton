#include "pos.h"

#include "engio.h" // for calls inside from_fen()
#include "piece.h"
#include "mqueue.h"
#include "data.h"

#ifdef TEST

#include "debug.h"

#endif 

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


// to use the x86 bsr instruction 
#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))


void free_pos(struct position* pos) {

    free_piece(pos->pawns[WHITE]); free_piece(pos->pawns[BLACK]);
    free_piece(pos->hoppers[WHITE]); free_piece(pos->hoppers[BLACK]);
    free_piece(pos->sliders[WHITE]); free_piece(pos->sliders[BLACK]);
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


struct position* from_fen(char* fen) {

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
	    pos->pin_paths[sqr] = 0; 
	    
	    if (id == WHITE_KING) {
		if (pos->hoppers[WHITE] != NULL) {
		    printf("Invalid fen: more than 1 white king found. Exit.");
		    return NULL; 
		}
		pos->hoppers[WHITE] = p;
	    }
	    else if (id == BLACK_KING) {
		if (pos->hoppers[BLACK] != NULL) {
		    printf("Invalid fen: more than 1 black king found. Exit.");
		    return NULL; 
		}
		pos->hoppers[BLACK] = p;
	    }
	    	    
	    sqr++; 
	    index++;
	    continue; 
	}
	
	// else, we are left with the number of empty squares 
	U8 empty = fen[index] - '0';
	for (; empty > 0; empty--) {
	    pos->board[sqr] = NULL;
	    pos->pin_paths[sqr] = 0; 
	    sqr++; 
	}
	
	index++; 
    }

    if (pos->hoppers[WHITE] == NULL || pos->hoppers[BLACK] == NULL) {
	printf("Invalid fen: one or both kings missing. Exit");
	return NULL; 
    } 

    pos->board_mask = 0; 
    // kings were placed as heads of pos->hoppers' linked lists; adding the rest of the pieces 
    for (int i = 0; i < 64; i++) {
	
	if (pos->board[i] == NULL) continue;
	
	pos->board_mask |= (1ULL << i);
	
	enum piece_id id = pos->board[i]->id;
	if (id == WHITE_KING || id == BLACK_KING) continue;

	enum colour piece_colour = id < 6 ? WHITE : BLACK;
	U8 colour_offset = piece_colour == WHITE ? 0 : 6;

	pos->side_masks[piece_colour] |= (1ULL << 1);
	
	if (id - colour_offset == 2) 
			add_piece(pos, pos->board[i], pos->hoppers[piece_colour], i);
	else if (id - colour_offset == 1) 
			add_piece(pos, pos->board[i], pos->pawns[piece_colour], i);
	else 
			add_piece(pos, pos->board[i], pos->sliders[piece_colour], i);
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
	    else pos->castling_rights = read_fen_castle(fen, part_start, index);
	    break; 
	
	case 2:
	    if (fen[part_start] == '-') pos->en_passant_sqr = 64; 
	    else pos->en_passant_sqr = sqr_to_internal(&fen[part_start]); 
	    break; 
	} 
	
	part++; index++; part_start = index;  
    }

    return pos; 
}


char* to_fen(struct position* pos) {

    return NULL; 
}



// rating is 0 for now 
void bit_scan(const U8 sqr, const U64 moves, struct mqueue* queue) {
    
    I64 signed_moves = (I64)moves; 
    while (signed_moves != 0) {	
	I64 lsb = signed_moves & -signed_moves;
	U8 loglsb = (U8)LOG2((U64)lsb);
	enqueue(queue, get_move(sqr, loglsb, NORMAL, 0));
	signed_moves ^= lsb;
    }
}


// BLANK 

U64 static_attacks(){
	// update individual piece.attacks
}


U64 sliding_attacks(){
	// update individual piece.attacks
}


void pawn_moves(){
	// update individual piece.moves
}


void hopper_moves(){

	// update individual piece.moves
	// get the king first, then knights, so no need for case switches
}


void slider_moves(){
	// update individual piece.moves
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


// INCREMENTAL 

U64 merge_cached_attacks(struct piece* waiting) {

	// merge piece.attacks
}

U64 enqueue_cached_moves(struct piece* moving) {

	// merge piece.moves
	// many of these moves are already enqueued in the previous queue, which could perhaps yield
	// a significant optimisation if we can avoid enqueuing these (?)
}


// MAIN FUNCTIONS

enum board_state get_moves(struct position* pos, struct mqueue* queue) {
    
    const enum colour turn = pos->side;
    const enum colour wait = pos->side ^ 1;

    U8 state = 0;

    const U8 moving_king_sqr = pos->hoppers[turn]->sqr;
    const U64 waiting_side_mask = pos->side_masks[turn];
    const U64 moving_side_mask = pos->side_masks[turn ^ 1];
    const U64 board_mask = pos->board_mask;

    ///////////////////////////////////////////////// WAITING SIDE 

    const U64 moving_king_mask = 1ULL << moving_king_sqr;

    ///////////////////////////////////////////////// MOVING SIDE 
	
    return (enum board_state)state;
    
}


// move-making function 
enum board_state update_pos(struct position* pos, struct mqueue* queue, U32 move) {

    enum board_state state = QUIET;

    return state;
}



