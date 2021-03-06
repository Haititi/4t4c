#ifndef MOUSEDEVICE_H
#define MOUSEDEVICE_H

#include "Headers.h"
#include "Sprite.h"
#include "Events.h"
#include "SkinLoader.h"

struct InputMessage
{
	const unsigned int MessageType;
	const unsigned int Param1,Param2;
	const float TimeStamp;
	InputMessage(const unsigned int pMessageType,const unsigned int pParam1,const unsigned int pParam2,const float pTimeStamp)
		:MessageType(pMessageType),Param1(pParam1),Param2(pParam2),TimeStamp(pTimeStamp)
	{};
	InputMessage(const InputMessage& Ref)
		:MessageType(Ref.MessageType),Param1(Ref.Param1),Param2(Ref.Param2),TimeStamp(Ref.TimeStamp)
	{};
	InputMessage& operator=(const InputMessage& Ref)
	{
		//HACK to copy const member
		*(const_cast<unsigned int*>(&MessageType)) = Ref.MessageType;
		*(const_cast<unsigned int*>(&Param1)) = Ref.Param1;
		*(const_cast<unsigned int*>(&Param2)) = Ref.Param2;
		*(const_cast<float*>(&TimeStamp)) = Ref.TimeStamp;
		return *this;
	};
};

struct MouseState 
{
    int    X;
    int    Y;
	MouseState(void){X=0;Y=0;};
	MouseState(const int Px,const int Py){X=Px;Y=Py;};
};
typedef MouseState* PMouseState;

class InputManager
{
private:
	//keys section
	unsigned long ShiftState,AltState,CtrlState;
	std::vector<InputMessage> InputMessageList;

	//mouse section
	void* LockCustomPtr[4];
	unsigned int (* LockEventCallBack[4])(void*);
	unsigned long LockEvent[4];

	bool MouseDown;
	MouseState DragState;

	float ButtonTimer[8]; //timers to calculate double click 


	CriticalSection Lock;

	Event_DragObject* DragObj;
	bool Dragging;
	bool DragDetect(void); //Detect dragging

	UI_Base* OverUI;
	UI_Base* CaptureUI;

	InputManager(void);
public:
	int PosX;
	int PosY;

	static InputManager& Inst(void);
	
	~InputManager(void);
	void Initialize(void);

	MouseState GetMouseState(void){return MouseState(PosX,PosY);};

	bool LockNextEvent(unsigned long, LPVOID, unsigned int (* CallBack)(void*), int Button);
	void SetPosition(const int NewPosX,const int NewPosY);

	void SetOverUI(UI_Base* GameUI);
	void SetMouseCapture(UI_Base* GameUI){CaptureUI=GameUI;};

	//from keyboard.h
	void StoreMessage(const unsigned int MessageType,const unsigned int Param1,const unsigned int Param2);
	void ProcessMessageList(void);

	unsigned long GetCtrlState(void){return CtrlState;};
	unsigned long GetAltState(void){return AltState;};
	unsigned long GetShiftState(void){return ShiftState;};
};

class MouseManager 
{
private:
	SkinRenderer *CustomCursor;
	bool Correction;
	int XCor;
	int YCor;


public:
	MouseManager(void);
	~MouseManager(void);
	void Initialize(void);
	void SetCustomCursor(SkinRenderer *NewCursor, int x = 0, int y = 0, bool correction = false );
	void ClearCustomCursor(void); 
	void DrawCursor(const float DeltaTime,const int x,const int y);
};

extern MouseManager MouseCursor;
#endif
