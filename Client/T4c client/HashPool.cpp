#include "HashPool.h"
#include "hash.h"
#include <stdlib.h>

THashPool::THashPool(unsigned long PoolSize)
{
	//TODO we should choose the next prime bigger than PoolSize
	HashSize=PoolSize;
	ItemCount=0;
	HashList=new PHashItem[HashSize];

	ActualData=0;
	CycleIt=0;
	CycleObj=0;

	for(unsigned int i=0;i<HashSize;i++)
	{
		HashList[i]=0;
	}
};

THashPool::~THashPool(void)
{
	//go through each bucket and destroy each HashItem 
	for (unsigned int i=0;i<HashSize;i++)
	{
	}

	delete [] HashList;
};

bool THashPool::AddHashEntry(const unsigned long Hashcode,void* Data)
{

	PHashItem Last=0;
	PHashItem *ActEntry;

	const unsigned long Num=Hashcode%HashSize;
	
	ActEntry=&HashList[Num];
	while(1)
	{
		if (!(*ActEntry)) 
		{
			*ActEntry=new THashItem;
			(*ActEntry)->Data=Data;
			(*ActEntry)->Hash=Hashcode; 
			(*ActEntry)->Next=0;
			if (Last)
				Last->Next=*ActEntry;
			ItemCount++;
			return true;
		} else
		{
			if ((*ActEntry)->Hash==Hashcode)  //collision : very very rare
				return false;
			Last=*ActEntry;
			ActEntry=&(*ActEntry)->Next;
		}
	}
};

bool THashPool::ReplaceHashEntry(const unsigned long Hashcode,void* Data)
{
	PHashItem ActEntry;

	const unsigned long Num=Hashcode%HashSize;
	
	ActEntry=HashList[Num];
	while(1)
	{
		if (!ActEntry) 
		{
			return false;
		} else
		{
			if (ActEntry->Hash==Hashcode)  //collision
			{
				ActEntry->Data=Data;
				return true;
			}
			ActEntry=ActEntry->Next;
		}
	}
};

void* THashPool::GetEntry(const unsigned long Hashcode)
{
	PHashItem ActEntry=0;

	const unsigned long Num=Hashcode%HashSize;

	ActEntry=HashList[Num];

	while (1)
	{
		if (!ActEntry) 
			return 0;
		if (ActEntry->Hash==Hashcode) 
			return ActEntry->Data;
		ActEntry=ActEntry->Next;
	}
};

bool THashPool::RemoveItem(const unsigned long Hashcode)
{
	PHashItem ActEntry=0,LastEntry=0;

	const unsigned long Num=Hashcode%HashSize;

	ActEntry=HashList[Num];

	while (1)
	{
		if (!ActEntry) 
			return false;
		if (ActEntry->Hash==Hashcode)
		{
			//We destroy that one
			if (LastEntry==0)
			{
				HashList[Num]=0;
			}else
			{
				if (ActEntry->Next!=0)
				{
					LastEntry->Next=ActEntry->Next;
				} else
				{
					LastEntry->Next=0;
				}
			}
			delete ActEntry;
			ItemCount--;
			return true;
		}
		LastEntry=ActEntry;
		ActEntry=ActEntry->Next;
	}
};

bool THashPool::IsObjectValid(void* Data)
{
	//TODO To Implement
	return true;
};
		
void THashPool::ResetCycling(void)
{
	CycleIt=0;
	CycleObj=HashList[0];
};
		
void* THashPool::GetNextEntry(void)
{
	//void* Result=0;
	ActualData=0;

	while ((ActualData==0) && (CycleIt!=(HashSize-1)))
	{
		if (CycleObj==0)
		{
			CycleIt++;
			CycleObj=HashList[CycleIt];
		} else
		{
			ActualData=CycleObj->Data;
			CycleObj=CycleObj->Next;
		}
	}
	return ActualData;
};