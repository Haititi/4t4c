#ifndef MOUSEDEVICE_H
#define MOUSEDEVICE_H

#include <windows.h>
#include <vector>
#include "TSprite.h"
#include "../newinterface/Events.h"
#include "SkinLoader.h"

//old defines for mouse lock
#define DM_NONE  0
#define DM_DOWN  1
#define DM_CLICK  2
#define DM_DOUBLE_CLICK  3
#define DM_UP  4
#define DM_DRAG  5
#define DM_DROP  6


struct TInputMessage
{
	unsigned int MessageType;
	unsigned int Param1,Param2;
	float TimeStamp;
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


	CRITICAL_SECTION InputCrit;

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
