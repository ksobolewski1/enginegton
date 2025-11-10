#pragma once 


/*

    Types and structs used throughout the program

*/


typedef unsigned long long int U64; 
typedef unsigned int U32; 
typedef unsigned short int U16;
typedef unsigned char U8;

typedef signed char I8;
typedef signed long long int I64; 


enum board_state { QUIET, CHECK, CHECKMATE, STALEMATE, INSUFFICIENT, THREEFOLD, FIFTY, ERROR_STATE };


enum colour { BLACK, WHITE };


enum piece_id { 
    WHITE_KING, WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, 
    BLACK_KING, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, 
    NONE 
};

enum move_type {
    NORMAL, PROMOTION, EN_PASSANT, CASTLE, CAPTURE
};
    
static inline U32 get_move(
    const U8 origin, // 8 bits
    const U8 destination, // 8 bits 
    const U8 move_type, // 4 bits 
    const U16 rating // 12 bits 
    ) {
    return (origin << 24) | (destination << 16) | (move_type << 12) | rating;  
}


static inline U8 origin(const U32 move) {
    return (U8)(move >> 24); 
}

static inline U8 destination(const U32 move) {
    return (U8)((move >> 16) & 0xFF); 
}

static inline U8 move_type(const U32 move) {
    return (U8)((move >> 12) & 0xF); 
}

static inline U16 rating(const U32 move) {
    return (U16)(move & 0xFFFF); 
}

static inline U16 get_move16(U8 origin, U8 destination) {
    return (origin << 8) | destination; 
}

static inline U8 origin16(const U16 move) {
    return (U8)(move >> 8); 
}

static inline U8 destination16(const U16 move) {
    return (U8)(move & 0xFF); 
}

static inline U16 move_lts(const U32 m) {
    return (U16)(m >> 16); 
}

// short for "mask-to-ones": output the mask itself if at least one bit set, else 0xFFFFFFFFFFFFFFFF
static inline U64 mto(U64 mask) {
    return mask | (-!mask);
}

// short for "mask-to-zeroes": output 0xFFFFFFFFFFFFFFFF if at least one bit set, else 0
static inline U64 mtz(U64 mask) {
    return -!!mask;
}