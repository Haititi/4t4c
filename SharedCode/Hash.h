#ifndef HASH_H
#define HASH_H

#include <string>

void InitRandHash(void);

unsigned long RandHash(const char *Txt);
unsigned long RandHash(const unsigned long Value);
unsigned long RandHash(const std::wstring& Txt);



#endif