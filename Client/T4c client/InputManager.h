#ifndef MOUSEDEVICE_H
#define MOUSEDEVICE_H

#include "Headers.h"
#include "TSprite.h"
#include "../interface/Events.h"
#include "SkinLoader.h"

struct TInputMessage
{
	const unsigned int MessageType;
	const unsigned int Param1,Param2;
	const float TimeStamp;
	TInputMessage(const unsigned int pMessageType,const unsigned int pParam1,const unsigned int pParam2,const float pTimeStamp)
		:MessageType(pMessageType),Param1(pParam1),Param2(pParam2),TimeStamp(pTimeStamp)
	{};
	TInputMessage(const TInputMessage& Ref)
		:MessageType(Ref.MessageType),Param1(Ref.Param1),Param2(Ref.Param2),TimeStamp(Ref.TimeStamp)
	{};
	TInputMessage& operator=(const TInputMessage& Ref)
	{
		//HACK to copy const member
		*(const_cast<unsigned int*>(&MessageType)) = Ref.MessageType;
		*(const_cast<unsigned int*>(&Param1)) = Ref.Param1;
		*(const_cast<unsigned int*>(&Param2)) = Ref.Param2;
		*(const_cast<float*>(&TimeStamp)) = Ref.TimeStamp;
		return *this;
	};
};

struct TMouseState 
{
    int    X;
    int    Y;
};
typedef TMouseState* PMouseState;

class TInputManager
{
private:
	//keys section
	unsigned long ShiftState,AltState,CtrlState;
	std::vector<TInputMessage> InputMessageList;

	//mouse section
	void* LockCustomPtr[4];
	unsigned int (* LockEventCallBack[4])(void*);
	unsigned long LockEvent[4];

	bool MouseDown;
	TMouseState DragState;

	float ButtonTimer[8]; //timers to calculate double click 


	CriticalSection Lock;

	TDragObject* DragObj;
	bool Dragging;
	bool DragDetect(void); //Detect dragging

	TGameUI* OverUI;
	TGameUI* CaptureUI;
public:
	int PosX;
	int PosY;

	TInputManager(void);
	~TInputManager(void);
	void Initialize(void);

	bool LockNextEvent(unsigned long, LPVOID, unsigned int (* CallBack)(void*), int Button);
	void SetPosition(const int NewPosX,const int NewPosY);

	void SetOverUI(TGameUI* GameUI);
	void SetMouseCapture(TGameUI* GameUI){CaptureUI=GameUI;};

	//from keyboard.h
	void StoreMessage(const unsigned int MessageType,const unsigned int Param1,const unsigned int Param2);
	void ProcessMessageList(void);

	unsigned long GetCtrlState(void){return CtrlState;};
	unsigned long GetAltState(void){return AltState;};
	unsigned long GetShiftState(void){return ShiftState;};
};

extern TInputManager Input; 

class TMouseCursor 
{
private:
	TSkinRenderer *CustomCursor;
	bool Correction;
	int XCor;
	int YCor;


public:
	TMouseCursor(void);
	~TMouseCursor(void);
	void Initialize(void);
	void SetCustomCursor(TSkinRenderer *NewCursor, int x = 0, int y = 0, bool correction = false );
	void ClearCustomCursor(void); 
	void DrawCursor(const float DeltaTime,const int x,const int y);
};

extern TMouseCursor MouseCursor;
#endif
