#include "Hash.h"

static unsigned long RndTable[256];
bool Initialized=false;

void InitRandHash(void)
{
	unsigned long i,RndSeed;
	
	if (Initialized)
		return;

	RndSeed=0xd2a9f7bd;
	for (i=0;i<256;i++)
	{
		RndSeed=RndSeed*0x08088405+1;
		RndTable[i]=RndSeed;
	}

	Initialized=true;
};

unsigned long RandHash(const char *Txt)
{
	unsigned long i,Result;
	const char* p;

	p=Txt;
	
	unsigned int Size=strlen(Txt);
	Result=0xFFFFFFFF;

	for (i=0;i<Size;i++)
	{
		Result=((Result << 5)+Result)^RndTable[(unsigned char)*p];
		p++;
	}
	return Result;
};

unsigned long RandHash(const unsigned long Value)
{
	unsigned long Result=0xFFFFFFFF;

	Result=((Result<<5)+Result)^RndTable[(Value)    &0xFF];
	Result=((Result<<5)+Result)^RndTable[(Value>>8) &0xFF];
	Result=((Result<<5)+Result)^RndTable[(Value>>16)&0xFF];
	Result=((Result<<5)+Result)^RndTable[(Value>>24)&0xFF];

	return Result;
}