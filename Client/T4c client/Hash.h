#ifndef HASH_H
#define HASH_H

#include "Headers.h"

void InitRandHash(void);

unsigned long RandHash(const char *Txt);
unsigned long RandHash(const unsigned long Value);



#endif