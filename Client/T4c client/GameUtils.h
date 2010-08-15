#ifndef GAMEUTILS_H
#define GAMEUTILS_H

#include <stdlib.h>
#define Pi (float)3.1415926535
#define Rad2Deg (float)180.0f/Pi
#define PiDiv2 (float)Pi/2.0f
#define PiDiv3 (float)Pi/3.0f

inline int Abs(int Value) 
{
	static const int INT_BITS = sizeof(int) * 8;
    int TopBit = Value >> (INT_BITS - 1);
    return (Value ^ TopBit) - TopBit;
}

//rounding float->int delphi style
static inline int Round (float const x) 
{ // Round to nearest integer
#ifdef _AMD64
 return (int) x;
#else
	int n;
	__asm fld dword ptr x;
	__asm fistp dword ptr n;
	return n;
#endif
};

//int Random(unsigned int n){return rand()%n};


float GetDistance(const int X1,const int Y1,const int X2,const int Y2);

float ComputeAngle(const float x,const float y);

int Angle2Dir(int Ang); //take an angle in degree

int RadToDeg(const float n);

//the function that pack all in one :

//network packet direction
int ComputeDirection(const int x,const int y); 
int ComputeDirection(const float x,const float y);

//Sprite Direction
int ComputeSpriteDirection(const int x,const int y);

inline unsigned long Random(const unsigned long n){return rand()%n;};

//for n=32  return a -32 -> 32  int with a pseudo gaussian distribution
int GaussRandom(const unsigned long n);


//random float functions
float RandFloat01(void);  //between  0 ->1
float RandFloat1m1(void); //between -1 ->1



#endif