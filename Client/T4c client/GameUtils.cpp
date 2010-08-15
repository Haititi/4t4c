#include "gameUtils.h"
#define _USE_MATH_DEFINES
#include <Math.h>

float GetDistance(const int X1,const int Y1,const int X2,const int Y2)
{
	return sqrtf( (float)((X2-X1)*(X2-X1)+(Y2-Y1)*(Y2-Y1)) );
};

float ComputeAngle(const float x,const float y)
{
	return (atan2(x,y)+(float)M_PI);
};

int Angle2Dir(int Ang)
{
	while(Ang>=360)
		Ang-=360;
	while(Ang<0)
		Ang+=360;
	const int DirTab[9]= {22,67,112,157,202,247,292,337,360};
	const int DirTab2[9]={1,8,7,6,5,4,3,2,1};
	for (int i=0;i<9;i++)
	{
		if(Ang<DirTab[i]) 
			return DirTab2[i];
	}
	return 1;
};

int Angle2DirSpr(const int Ang)
{
	const int DirTab[9]= {22,67,112,157,202,247,292,337,360};
	const int DirTab2[9]={4,3,2,1,0,7,6,5,4};
	for (int i=0;i<9;i++)
	{
		if(Ang<DirTab[i]) 
			return DirTab2[i];
	}
	return 1;
};

int RadToDeg(const float n)
{
	return int ((n*180.0f)/(float)M_PI);
};

int ComputeDirection(const int x,const int y)
{
	return Angle2Dir(RadToDeg(ComputeAngle((float)x,(float)y)));
};

int ComputeDirection(const float x,const float y)
{
	return Angle2Dir(RadToDeg(ComputeAngle(x,y)));
};

int ComputeSpriteDirection(const int x,const int y)
{
	int Ang=RadToDeg(ComputeAngle((float)x,(float)y));
	//secu? useless?
	if (Ang>=360)
 		Ang-=360;
	if (Ang<0)
		Ang+=360;
	return Angle2DirSpr(Ang);
};

int GaussRandom(const unsigned long n)
{
	const unsigned long n2=n<<1;
	return ((rand()%n2)+(rand()%n2)-(rand()%n2)-(rand()%n2)) / 4;
};

unsigned long FloatRandSeed=0;

unsigned long FloatRandInt(void)
{
	FloatRandSeed=FloatRandSeed* 214013L + 2531011L;
	return (FloatRandSeed>>8) & 0x007fffff; //more entropy in the higher bytes
};

float RandFloat01(void)
{
	//generate a mantissa (23 bits) //range 0.0f->1.0f
	unsigned long Value=FloatRandInt() | 0x3f000000;
	//reinterpret as float
	return *((float*)(&Value)); 
};

 //return( ((ptd->_holdrand = ptd->_holdrand * 214013L + 2531011L) >> 16) & 0x7fff );

float RandFloat1m1(void)
{
	//generate a mantissa (23 bits) //range 2.0f->4.0f  by setting exponent to 1
	unsigned long Value=FloatRandInt() | 0x40000000; 
	//reinterpret as float
	return *((float*)(&Value))-3.0f; 
};