#include "pos.h"

#include "engio.h" // for calls inside from_fen()
#include "piece.h"
#include "mqueue.h"

#include <stdlib.h>


// for promotion and reassigning the funcs, we just make an array of 0-3 indexed att and move funcs 


void free_pos(struct position* pos) {

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

	enum colour pcolour = id < 6 ? WHITE : BLACK;
	U8 colour_offset = pcolour == WHITE ? 0 : 6;

	pos->side_masks[pcolour] |= (1ULL << 1);
	
	if (id - colour_offset < 3) add_piece(pos, pos->board[i], pos->hoppers[pcolour], i);
	else add_piece(pos, pos->board[i], pos->sliders[pcolour], i);
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


