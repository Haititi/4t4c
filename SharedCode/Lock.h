#ifndef LOCK_H
#define LOCK_H

#include "Headers.h"

class CriticalSection
{
private:
    CRITICAL_SECTION CritSect;

public:
    CriticalSection(void)
	{ 
        InitializeCriticalSection( &CritSect ); 
    };
    ~CriticalSection(void)
	{
        DeleteCriticalSection( &CritSect );
    };


    void Lock(void)
	{    
        EnterCriticalSection( &CritSect );  
    };

    void Unlock(void)
	{    
        LeaveCriticalSection( &CritSect );
    }
      
    bool TryLock(void)
	{
        return TryEnterCriticalSection( &CritSect )!=0;
    }
};

class ScopedLock 
{
private:
	CriticalSection& Lock;
public:
	ScopedLock(CriticalSection &pLock):Lock(pLock)
	{
		Lock.Lock();
	};
	~ScopedLock(void)
	{
		Lock.Unlock();
	};
};

#endif 