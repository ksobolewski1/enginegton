
#include "core.h"

#include "mt19937.h"

#include <stdlib.h>

// these arrays are needed in magic()
U64 rook_masks[64] = {0};
U64 bishop_masks[64] = {0};
U64 rook_configs[64][4096] = {0};
U64 bishop_configs[64][512] = {0};

const U8 rook_index_shifts[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};
const U8 bishop_index_shifts[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};


void write64(FILE* fptr, const char* name, U64(*gen)(U8, U8, U8)); 

void write8(FILE* fptr, const char* name, const U8* arr);

void write_magics(FILE* fptr, const char* name, U8 rook);

void write_configs(FILE* fptr);

void write_king_rays(FILE* fptr);


U64 king_attack(U8 y, U8 x, U8 origin);

U64 wpawn_attack(U8 y, U8 x, U8 origin);

U64 bpawn_attack(U8 y, U8 x, U8 origin);

U64 knight_attack(U8 y, U8 x, U8 origin);

U64 rook_attack(U8 y, U8 x, U8 origin);

U64 bishop_attack(U8 y, U8 x, U8 origin);
    
U64 rook_mask(U8 y, U8 x, U8 origin);

U64 bishop_mask(U8 y, U8 x, U8 origin);

U64 rook_config(U8 origin, U64 blockers);

U64 bishop_config(U8 origin, U64 blockers);

U64 magic(U8 origin, U8 rook);


int main(int argc, char** argv) {

    if (argc < 2) {
	
	    return 1;
    }
 
    FILE* fptr;
    fptr = fopen(argv[1], "w");
    if (fptr == NULL) {
	fprintf(stderr, "Error: Failed to create data.c\n");
	return 1; 
    }
    fprintf(fptr, "#include \"data.h\"\n"); 

    // const U64[64] and const U8[64]
    printf("Generating attacks and masks...\n");
    write64(fptr, "king_attacks", king_attack);
    write64(fptr, "wpawn_attacks", wpawn_attack);
    write64(fptr, "bpawn_attacks", bpawn_attack);
    write64(fptr, "knight_attacks", knight_attack);
    write8(fptr, "rook_shifts", rook_index_shifts);
    write8(fptr, "bishop_shifts", bishop_index_shifts);
    write64(fptr, "rook_attacks", rook_attack);
    write64(fptr, "bishop_attacks", bishop_attack);
    write64(fptr, "rook_masks", rook_mask);
    write64(fptr, "bishop_masks", bishop_mask);
    
    // magics (also defines rook and bishop configs)
    printf("Generating rook and bishop magic numbers...\n");
    write_magics(fptr, "rook_magics", 1);
    write_magics(fptr, "bishop_magics", 0);

    // all configurations of blockers for sliding pieces
    printf("Writing rook and bishop attacks...\n");
    write_configs(fptr);

    write_king_rays(fptr);

    // random zob numbers
    // evaluation tables
    
    fclose(fptr);

    return 0; 
} 

void write64(FILE* fptr, const char* name, U64(*get)(U8, U8, U8)) {
    
    fprintf(fptr, "const U64 %s[64] = {\n", name);
    
    for (U8 i = 0; i < 64; i++) {

	U8 file = i % 8;
	U8 rank = (i - file) >> 3;
	
	fprintf(fptr, "%lluu, ", get(rank, file, i));
	if (i % 8 == 7) fprintf(fptr, "\n");
	
    }
    
    fprintf(fptr, "};\n");     
}

void write8(FILE* fptr, const char* name, const U8* arr) {

    fprintf(fptr, "const U8 %s[64] = {\n", name);
    for (U8 i = 0; i < 64; i++) {
	fprintf(fptr, "%du, ", arr[i]);
	if (i % 8 == 7) fprintf(fptr, "\n");
    }
    fprintf(fptr, "};\n");      
}

void write_magics(FILE* fptr, const char* name, U8 rook) {
    fprintf(fptr, "const U64 %s[64] = {\n", name);
    for (U8 i = 0; i < 64; i++) {
	fprintf(fptr, "%lluu, ", magic(i, rook));
	if (i % 8 == 7) fprintf(fptr, "\n");
    }
    fprintf(fptr, "};\n"); 
}


void write_configs(FILE* fptr) {

    fprintf(fptr, "const U64 rook_configs[64][4096] = {\n");
    for (U8 i = 0; i < 64; i++) {
	fprintf(fptr, "{");
	for (U16 j = 0; j < 4096; j++) {
	    fprintf(fptr, "%lluu, ", rook_configs[i][j]);
	    if (j % 8 == 7) fprintf(fptr, "\n");
	}
	fprintf(fptr, "},\n");
    }
    fprintf(fptr, "};\n");

    
    fprintf(fptr, "const U64 bishop_configs[64][512] = {\n");
    for (U8 i = 0; i < 64; i++) {
	fprintf(fptr, "{");
	for (U16 j = 0; j < 512; j++) {
	    fprintf(fptr, "%lluu, ", bishop_configs[i][j]);
	    if (j % 8 == 7) fprintf(fptr, "\n");
	}
	fprintf(fptr, "},\n");
    }
    fprintf(fptr, "};\n"); 
}


void write_king_rays(FILE* fptr) {

    fprintf(fptr, "const U64 king_rays[64][9] = {\n");

    // ray by ray 

    for (U8 i = 0; i < 64; i++) {

        fprintf(fptr, "{");
        
        U8 x = i % 8;
        U8 y = (i - x) >> 3;

        // rank is y
        // file is x

        U64 n, s, e, w, nw, sw, se, ne;
        n = w = e = s = nw = sw = se = ne = 0;

        for (I8 i = y - 1; i >= 0; i--) n |= (1ULL << (i * 8 + x));
        for (U8 i = y + 1; i < 8; i++) s |= (1ULL << (i * 8 + x)); 
        for (U8 i = x + 1; i < 8; i++) e |= (1ULL << (y * 8 + i)); 
        for (I8 i = x - 1; i >= 0; i--) w |= (1ULL << (y * 8 + i));  

        for (I8 i = y - 1, j = x + 1; i >= 0 && j < 8; i--, j++) ne |= (1ULL << (i * 8 + j));
        for (I8 i = y + 1, j = x + 1; i < 8 && j < 8; i++, j++) se |= (1ULL << (i * 8 + j));
        for (I8 i = y - 1, j = x - 1; i >= 0 && j >= 0; i--, j--) nw |= (1ULL << (i * 8 + j));
        for (I8 i = y + 1, j = x - 1; i < 8 && j >= 0; i++, j--) sw |= (1ULL << (i * 8 + j));

        fprintf(fptr, "%lluu, %lluu, %lluu, %lluu, 0, %lluu, %lluu, %lluu, %lluu, ", nw, n, ne, w, e, sw, s, se);

        fprintf(fptr, "},\n");
    }

    fprintf(fptr, "};\n"); 
}


U64 king_attack(U8 y, U8 x, U8 origin) {
    
    U64 control = 0;

    if (x + 1 < 8) control |= (1ULL << (origin + 1));
    if (x - 1 >= 0) control |= (1ULL << (origin - 1));
    if (y - 1 >= 0) control |= (1ULL << (origin - 8));
    if (y + 1 < 8) control |= (1ULL << (origin + 8));
    if (x - 1 >= 0 && y - 1 >= 0) control |= (1ULL << (origin - 9));
    if (x + 1 < 8 && y + 1 < 8) control |= (1ULL << (origin + 9));
    if (x - 1 >= 0 && y + 1 < 8) control |= (1ULL << (origin + 7));
    if (x + 1 < 8 && y - 1 >= 0) control |= (1ULL << (origin - 7));

    return control;
}

U64 wpawn_attack(U8 y, U8 x, U8 origin) {
    U64 control = 0;
    if (origin <= 7 || origin >= 56) return control; 
    if (x + 1 < 8) control |= (1ULL << (origin - 7));
    if (x - 1 >= 0) control |= (1ULL << (origin - 9));
    return control;
}

U64 bpawn_attack(U8 y, U8 x, U8 origin) {
    U64 control = 0;
    if (origin <= 7 || origin >= 56) return control; 
    if (x - 1 >= 0) control |= (1ULL << (origin + 7));
    if (x + 1 < 8) control |= (1ULL << (origin + 9));
    return control;
}

U64 knight_attack(U8 y, U8 x, U8 origin) {
    U64 control = 0;
    
    if (x + 2 < 8 && y + 1 < 8) control |= (1ULL << (origin + 10));
    if (x - 2 >= 0 && y + 1 < 8) control |= (1ULL << (origin + 6));
    if (x + 2 < 8 && y - 1 >= 0) control |= (1ULL << (origin - 6));
    if (x - 2 >= 0 && y - 1 >= 0) control |= (1ULL << (origin - 10));
    if (y + 2 < 8 && x + 1 < 8) control |= (1ULL << (origin + 17));
    if (y - 2 >= 0 && x + 1 < 8) control |= (1ULL << (origin - 15));
    if (y + 2 < 8 && x - 1 >= 0) control |= (1ULL << (origin + 15));
    if (y - 2 >= 0 && x - 1 >= 0) control |= (1ULL << (origin - 17));
    
    return control;
}


// NOTE: iterator is signed in >=0 loops in rook_attack, bishop_attack, rook_config, bishop_config

U64 rook_attack(U8 y, U8 x, U8 origin) {
    
    U64 control = 0;

    for (I8 i = y - 1; i >= 0; i--) control |= (1ULL << (i * 8 + x));
    for (U8 i = y + 1; i < 8; i++) control |= (1ULL << (i * 8 + x));
    for (U8 i = x + 1; i < 8; i++) control |= (1ULL << (y * 8 + i));
    for (I8 i = x - 1; i >= 0; i--) control |= (1ULL << (y * 8 + i));
	
    return control;
}

U64 bishop_attack(U8 y, U8 x, U8 origin) {
	
    U64 control = 0;

    for (I8 i = y - 1, j = x + 1; i >= 0 && j < 8; i--, j++) control |= (1ULL << (i * 8 + j));
    for (I8 i = y + 1, j = x + 1; i < 8 && j < 8; i++, j++) control |= (1ULL << (i * 8 + j));
    for (I8 i = y - 1, j = x - 1; i >= 0 && j >= 0; i--, j--) control |= (1ULL << (i * 8 + j));
    for (I8 i = y + 1, j = x - 1; i < 8 && j >= 0; i++, j--) control |= (1ULL << (i * 8 + j));
	
    return control;

}

U64 rook_mask(U8 y, U8 x, U8 origin) {
  
    U64 control = 0;

    for (I8 i = y - 1; i >= 1; i--) control |= (1ULL << (i * 8 + x));
    for (U8 i = y + 1; i < 7; i++) control |= (1ULL << (i * 8 + x));
    for (U8 i = x + 1; i < 7; i++) control |= (1ULL << (y * 8 + i));
    for (I8 i = x - 1; i >= 1; i--) control |= (1ULL << (y * 8 + i));
    
    rook_masks[origin] = control;
    return control;
}

U64 bishop_mask(U8 y, U8 x, U8 origin) {

    U64 control = 0;

    for (I8 i = y - 1, j = x + 1; i >= 1 && j < 7; i--, j++) control |= (1ULL << (i * 8 + j));
    for (U8 i = y + 1, j = x + 1; i < 7 && j < 7; i++, j++) control |= (1ULL << (i * 8 + j));
    for (I8 i = y - 1, j = x - 1; i >= 1 && j >= 1; i--, j--) control |= (1ULL << (i * 8 + j));
    for (I8 i = y + 1, j = x - 1; i < 7 && j >= 1; i++, j--) control |= (1ULL << (i * 8 + j));
    
    bishop_masks[origin] = control;
    return control;
}


U64 rook_config(U8 origin, U64 blockers) {

    U8 x = origin % 8;
    U8 y = (origin - x) >> 3;
    
    U64 config = 0;
    
    for (I8 i = y - 1; i >= 0; i--) {
	U64 next = (1ULL << (i * 8 + x));
	config |= next;
	if ((blockers & next) > 0) break;
    }
    
    for (U8 i = y + 1; i < 8; i++) {
	U64 next = (1ULL << (i * 8 + x));
	config |= next;
	if ((blockers & next) > 0) break;
    }
    
    for (U8 i = x + 1; i < 8; i++) {
	U64 next = (1ULL << (y * 8 + i));
	config |= next;
	if ((blockers & next) > 0) break;
    }
    
    for (I8 i = x - 1; i >= 0; i--) {
	U64 next = (1ULL << (y * 8 + i));
	config |= next;
	if ((blockers & next) > 0) break;
    }
    return config;
}

U64 bishop_config(U8 origin, U64 blockers) {

    U8 x = origin % 8;
    U8 y = (origin - x) >> 3;
    
    U64 config = 0;
    
    for (I8 i = y - 1, j = x + 1; i >= 0 && j < 8; i--, j++) {
	U64 next = 1ULL << (i * 8 + j);
	config |= next;
	if ((blockers & next) > 0) break;
    }
    
    for (U8 i = y + 1, j = x + 1; i < 8 && j < 8; i++, j++) {
	U64 next = 1ULL << (i * 8 + j);
	config |= next;
	if ((blockers & next) > 0) break;
    }
    
    for (I8 i = y - 1, j = x - 1; i >= 0 && j >= 0; i--, j--) {
	U64 next = 1ULL << (i * 8 + j);
	config |= next;
	if ((blockers & next) > 0) break;
    }
    
    for (I8 i = y + 1, j = x - 1; i < 8 && j >= 0; i++, j--) {
	U64 next = 1ULL << (i * 8 + j);
	config |= next;
	if ((blockers & next) > 0) break;
    }
    return config;
}


U64 magic(U8 origin, U8 rook) {
    
    U64 mask = rook ? rook_masks[origin] : bishop_masks[origin]; 
    U8 indices[14] = {0};
    U8 indices_len = 0;
    
    U8 bit = 0;
    while (mask > 0) {
	U64 move = mask & (1ULL << bit);
	if (move > 0) {
	    indices[indices_len++] = bit;
	    mask ^= move;
	}
	bit++; 
    }
    
    U16 num = (U16)(1 << indices_len); 
    U64* all_blocks = (U64*)malloc(sizeof(U64) * (num + 1));
    
    for (U16 i = 0; i < num; i++) {
	U64 blockers = 0;
	for (U16 j = 0; j < indices_len; j++) {
	    U64 bit = (i >> j) & 1;
	    blockers |= (bit << indices[j]);
	}
	all_blocks[i] = blockers;
    }

    U64 magic;
    
    while (1) {

	magic = genrand64_int64() & genrand64_int64() & genrand64_int64();
	
	U8 taken[4096] = {0};
	U16 max = rook ? 4095 : 511;
	
	for (U16 i = 0; i < num; i++) {
	    
	    U16 key = rook ?
		(all_blocks[i] * magic) >> (64 - rook_index_shifts[origin])
		: (all_blocks[i] * magic) >> (64 - bishop_index_shifts[origin]);

	    if (key > max || taken[key]) break;
	    
	    if (rook) rook_configs[origin][key] = rook_config(origin, all_blocks[i]);
	    else bishop_configs[origin][key] = bishop_config(origin, all_blocks[i]);

	    taken[key] = 1;
	    
	    if (i == num - 1) goto END;
	}
  }

END: 
    free(all_blocks); 
    return magic;
}
