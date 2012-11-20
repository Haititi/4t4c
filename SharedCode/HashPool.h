#ifndef HASHPOOL_H
#define HASHPOOL_H

//store a pointer  related to an associated Key

struct HashItem
{
	unsigned long Hash;
	void *Data;
	HashItem *Next;
};

typedef HashItem* PHashItem;

class HashIterator;

//TODO the Hashpool eventually need a critical section
class HashPool
{
	friend HashIterator;
private:
	unsigned long HashSize; //size of the HashTable
	int ItemCount;
	PHashItem* HashList;

	void* ActualData;
public:
	HashPool(unsigned long PoolSize);
	~HashPool(void);
	bool AddHashEntry(const unsigned long Hashcode,void* Data);
	bool ReplaceHashEntry(const unsigned long Hashcode,void* Data);
	void* GetEntry(const unsigned long Hashcode);
	bool RemoveItem(const unsigned long Hashcode);
	bool IsObjectValid(void* Data);
	int Count(void){return ItemCount;};
	
	void InitIterator(HashIterator &It);
};

class HashIterator
{
	friend HashPool;
private:
	HashPool const * Parent;
	unsigned long CycleIt; // Entry number in the hash list
	PHashItem CycleObj;//Actual pointed item;
	void* ActualData;
public:
	HashIterator(void):Parent(0){};

	void ResetCycling(void);
	void* GetNextEntry(void);
	void* GetActualEntry(void){return ActualData;}; 
};


#endif