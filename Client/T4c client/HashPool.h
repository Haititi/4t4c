#ifndef HASHPOOL_H
#define HASHPOOL_H

//store a pointer  related to an associated Key

struct THashItem
{
	unsigned long Hash;
	void *Data;
	THashItem *Next;
};

typedef THashItem* PHashItem;


//TODO the Hashpool eventually need a critical section
class THashPool
{
	private:
		unsigned long HashSize; //size of the HashTable
		int ItemCount;
		PHashItem* HashList;

		unsigned long CycleIt; // Entry number in the hash list
		PHashItem CycleObj;//Actual pointed item;
		void* ActualData;
	public:
		THashPool(unsigned long PoolSize);
		~THashPool(void);
		bool AddHashEntry(const unsigned long Hashcode,void* Data);
		bool ReplaceHashEntry(const unsigned long Hashcode,void* Data);
		void* GetEntry(const unsigned long Hashcode);
		bool RemoveItem(const unsigned long Hashcode);
		bool IsObjectValid(void* Data);
		
		void ResetCycling(void);
		void* GetNextEntry(void);
		void* GetActualEntry(void){return ActualData;}; 
};


#endif