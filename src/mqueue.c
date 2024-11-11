#include "mqueue.h"

#include <stdio.h>
#include <stdlib.h>

/*

  for compatibility, data[0] is ignored in both HEAP and NOHEAP builds 
  
*/


struct mqueue* get_mqueue() {
    struct mqueue* q = (struct mqueue*)malloc(sizeof(struct mqueue));
    if (q == NULL) {
	fprintf(stderr, "Error: Failed to allocate memory for struct 'mqueue'");
	return NULL; 
    }
    q->N = 0; 
    for (int i = 0; i < 256; i++) q->data[i] = 0;
    return q; 
}


static inline void swap(U32* data, U8 i, U8 j) {
    U32 temp = data[i];
    data[i] = data[j];
    data[j] = temp; 
}


void sift_up(U32* data, const U32 N, U8 i) {
    
    if (i > N || i == 1) return;
	
    U32 parent_index = i >> 1;
    U32 m1 = data[parent_index];
    U32 m2 = data[i]; 
    
    if (rating(m2) > rating(m1)) swap(data, parent_index, i);
    sift_up(data, N, parent_index);    
}

void sift_down(U32* data, const U32 N, U8 i) {
    
    U8 prev = i;
    U8 left_index = i << 1;
    U8 right_index = (i << 1) + 1; 
    U32 current_move = data[i]; 
    
    if (rating(current_move) < rating(data[left_index]))
	prev = left_index;
    if (rating(data[right_index]) > rating(data[prev]))
	prev = right_index;
    
    if (prev != i && prev <= N) {
	swap(data, i, prev);
	sift_down(data, N, prev);
    }
}

U32 dequeue(struct mqueue* q) {
    swap(q->data, 1, q->N);
    U32 move = q->data[q->N--];
    sift_down(q->data, q->N, 1);
    return move;
}

void enqueue(struct mqueue* q, const U32 move) {
    
    q->data[q->N++] = move;
    U32 n = q->N;
    sift_up(q->data, n, n);

}


