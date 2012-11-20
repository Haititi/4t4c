#include "HashPool.h"
#include "hash.h"

HashPool::HashPool(unsigned long PoolSize)
{
	//TODO we should choose the next prime bigger than PoolSize
	HashSize=PoolSize;
	ItemCount=0;
	HashList=new PHashItem[HashSize];

	ActualData=nullptr;

	for(unsigned int i=0;i<HashSize;i++)
	{
		HashList[i]=nullptr;
	}
};

HashPool::~HashPool(void)
{
	//go through each bucket and destroy each HashItem 
	for (unsigned int i=0;i<HashSize;i++)
	{
	}

	delete [] HashList;
};

bool HashPool::AddHashEntry(const unsigned long Hashcode,void* Data)
{

	PHashItem Last=nullptr;
	PHashItem *ActEntry;

	const unsigned long Num=Hashcode%HashSize;
	
	ActEntry=&HashList[Num];
	while(1)
	{
		if (!(*ActEntry)) 
		{
			*ActEntry=new HashItem;
			(*ActEntry)->Data=Data;
			(*ActEntry)->Hash=Hashcode; 
			(*ActEntry)->Next=nullptr;
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

bool HashPool::ReplaceHashEntry(const unsigned long Hashcode,void* Data)
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

void* HashPool::GetEntry(const unsigned long Hashcode)
{
	PHashItem ActEntry=nullptr;

	const unsigned long Num=Hashcode%HashSize;

	ActEntry=HashList[Num];

	while (1)
	{
		if (!ActEntry) 
			return nullptr;
		if (ActEntry->Hash==Hashcode) 
			return ActEntry->Data;
		ActEntry=ActEntry->Next;
	}
};

bool HashPool::RemoveItem(const unsigned long Hashcode)
{
	PHashItem ActEntry=nullptr,LastEntry=nullptr;

	const unsigned long Num=Hashcode%HashSize;

	ActEntry=HashList[Num];

	while (1)
	{
		if (!ActEntry) 
			return false;
		if (ActEntry->Hash==Hashcode)
		{
			//We destroy that one
			if (LastEntry==nullptr)
			{
				HashList[Num]=nullptr;
			}else
			{
				if (ActEntry->Next!=nullptr)
				{
					LastEntry->Next=ActEntry->Next;
				} else
				{
					LastEntry->Next=nullptr;
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

bool HashPool::IsObjectValid(void* Data)
{
	//TODO To Implement
	return true;
};

void HashPool::InitIterator(HashIterator &It)
{
	It.Parent=this;
	It.ResetCycling();
};
		
void HashIterator::ResetCycling(void)
{
	CycleIt=0;
	CycleObj=Parent->HashList[0];
};
		
void* HashIterator::GetNextEntry(void)
{
	//void* Result=0;
	ActualData=nullptr;

	while ((ActualData==nullptr) && (CycleIt!=(Parent->HashSize-1)))
	{
		if (CycleObj==nullptr)
		{
			CycleIt++;
			CycleObj=Parent->HashList[CycleIt];
		} else
		{
			ActualData=CycleObj->Data;
			CycleObj=CycleObj->Next;
		}
	}
	return ActualData;
};