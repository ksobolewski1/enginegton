#pragma once

#include "core.h"

#define QHEAD 1 

struct mqueue {
    U8 N;
    U32 data[256]; 
};

struct mqueue* get_mqueue(); 

void enqueue(struct mqueue* q, const U32 move);

U32 dequeue(struct mqueue* q);

