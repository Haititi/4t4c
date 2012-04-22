#ifndef SYSMSG_H
#define SYSMSG_H

#include "Headers.h"

#include "Global.h"
#include "d3dfont.h"

class TSysMsg 
{
private:
    TD3dText *Text;
    float TimeStamp;
	unsigned long Color;
public:
    TSysMsg(void);
    ~TSysMsg(void);
    void SetText( std::wstring &Txt, const unsigned long BaseColor,const float Time,const bool MultiLine);
    void Draw(const int x,const int y ,const unsigned long Alpha);
    unsigned int GetHeight(void);
    float GetTimestamp(void){ return TimeStamp; };
};

class TSysMsgList
{
private:
	std::list<TSysMsg*> MsgList;
	//critical section to maybe remove later
	//AFTER in deepth inspection of code
	CriticalSection Lock;
public:
	TSysMsgList(void);
	~TSysMsgList(void);
	void AddMessage(std::wstring &Text, unsigned long Color, float Time, bool AllowNewLine);
	void Render(float DeltaTime);
};

extern TSysMsgList SysMsgList;


#endif