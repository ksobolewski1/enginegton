
#include "helpers.h"

#include "engio.h"
#include "mqueue.h"

#include <stdio.h>
#include <string.h> // for memcpy
#include <stdlib.h> 

// to sort moves 
void selection_sort(U32* arr, U8 len) {
    for (int i = 0; i < len; i++) {
	int n = i;
	for (int j = i+1; j < len; j++) {
	    if (arr[n] < arr[j]) n = j;  
	}
	U32 temp = arr[i];
	arr[i] = arr[n];
	arr[n] = temp;	    
    }    
} 


char** read_in(const char* filename, int line_count) {

    // intiialise the 2D array to store the fen 
    char** lines = (char**)malloc(sizeof(char*) * (line_count + 1));
    if (lines == NULL) {
	fprintf(stderr, "Error: Failed to allocate memory");
	return NULL; 
    }
	
    FILE* fptr;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    // open the file for read 
    fptr = fopen(filename, "r");
    if (fptr == NULL) {
	fprintf(stderr, "Error: Failed to open file %s", filename);
	return NULL; 
    }

    // read the lines off the file 
    int i = 0; 
    while ((read = getline(&line, &len, fptr)) != -1) {
	char* ln = (char*)malloc(sizeof(char) * (len + 1));
	memcpy(ln, line, len + 1);
	lines[i] = ln; 
	i++; 
    }

    fclose(fptr);
    if (line != NULL) free(line);
    
    return lines; 
}


U8 read_moves(const char* filename, struct mqueue** expects) {

    // open the file for read
    FILE* fptr;
    fptr = fopen(filename, "r");
    if (fptr == NULL) {
	fprintf(stderr, "Error: Failed to open file %s", filename);
	return 1; 
    }

    char* buff = NULL;
    if (fseek(fptr, 0L, SEEK_END) != 0) {
	fprintf(stderr, "Error: Failed to find EOF");
	return 1;
    }

    long fsize = ftell(fptr);
    if (fsize == -1) {
	fprintf(stderr, "Error: Incorrect buffer size");
	return 1;
    }
    
    buff = malloc(sizeof(char) * (fsize + 1));
    if (buff == NULL) {
	fprintf(stderr, "Error: Failed to allocate memory for the buffer");
	return 1;
    }
    
    if (fseek(fptr, 0L, SEEK_SET) != 0) {
	fprintf(stderr, "Error: (when returning to SET)");
	return 1; 
    }

    // get the length of the buffer and place a guard 
    size_t nlen = fread(buff, sizeof(char), fsize, fptr);
    buff[nlen++] = '\0'; 
   
    fclose(fptr);

    // convert the move to internal U32 representation and enqueue
    // every move is 4 chars long 
    U64 index = 0;
    U32 queue_index = 0; 
    while (buff[index] != '\0') {

	if (buff[index] == ';') {
	    queue_index++; index++;
	    continue; // means the new list of moves is here 
	}
	
	enqueue(expects[queue_index], move_to_internal(&buff[index])); 
	index+=4;
    }
    
    return 0; 
}


U8 verify_moves(struct mqueue* m, struct mqueue* expect) {

    U8 res = 0; 
    
    U32* m_data = m->data;
    U32* expect_data = expect->data;

    if (m->N != expect->N) return 1;  

    if (m->N > 0) selection_sort(m->data, m->N);  
    if (expect->N > 0) selection_sort(m->data, m->N);

    for (U32 i = 1; i <= m->N; i++) {
	if (m_data[i] != expect_data[i]) {
	    // print out the diff
	    res = 1; 
	} 
    }

    return res; 
}
