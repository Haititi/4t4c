#include "InputManager.h"
#include "Debug.h"
#include "GfxEngine.h" 
#include "SystemInfo.h"
#include "Netcore.h"
#include "LocStr.h"
#include "App.h"
#include "Global.h"
#include "./interface/MainUI.h"
#include "GameUtils.h"
#include "Config.h"
#include "TGameObject.h"
#include "macrohandler.h"
#include "GameTime.h"

//we include it here , not in headers.haaaaaaaa : it has some problematic macro
#include <WindowsX.h>

TInputManager Input;
TMouseCursor MouseCursor;

const int NbCursors=6;
unsigned long ActualCursor;

const float DblClickTime=0.2f;
const int DragDist=16;

//
TAnimSprite *Cursors[19];
TAnimSprite MouseCursor1; //norm
TAnimSprite MouseCursor2;
TAnimSprite MouseCursor3;
TAnimSprite MouseCursor5;//norm
TAnimSprite MouseCursor6;//norm
TAnimSprite MouseCursor7;//norm
TAnimSprite MouseCursor8;//norm
TAnimSprite MouseCursor9;//norm
TAnimSprite MouseCursor10;//norm
TAnimSprite MouseCursor11;//norm
TAnimSprite MouseCursor12;//norm
TAnimSprite MouseCursor16;
TAnimSprite MouseCursor17;
TAnimSprite MouseCursor18;

TAnimSprite AttackCursorIcon;




TInputManager::TInputManager( void )
{
	MouseDown=false;
	Dragging=false;
	ShiftState=0;
	AltState=0;
	CtrlState=0;
	PosX=0;
	PosY=0;
	DragObj=0;
	LockEvent[0]=LockEvent[1]=LockEvent[2]=LockEvent[3]=0;
	OverUI=0;
	for (unsigned int i=0;i<8;i++)
		ButtonTimer[i]=TimeStruct.GetGlobalTime();
}


TInputManager::~TInputManager( void )
{
}

bool TInputManager::DragDetect(void)
{
	return MouseDown && (abs((DragState.X-PosX)*(DragState.Y-PosY))>DragDist);
};

void TInputManager::StoreMessage(const unsigned int MessageType,const unsigned int Param1,const unsigned int Param2)
{
	ScopedLock Al(Lock);
	InputMessageList.push_back(TInputMessage(MessageType,Param1,Param2,TimeStruct.GetGlobalTime()));
};

void TInputManager::SetOverUI(TGameUI* GameUI)
{
	if (GameUI!=OverUI)
	{
		if (OverUI!=0)
		{
			TMouseEvent Event(EMouseEventType::Leave,PosX,PosY,false);
			//send a mouseLeave
			OverUI->ProcessEvent(&Event);
		}

		OverUI=GameUI;

		if (OverUI!=0)
		{
			TMouseEvent Event(EMouseEventType::Enter,PosX,PosY,false);
			//send a mouseLeave
			OverUI->ProcessEvent(&Event);
		}

	} else
	{
		//nothing happens
		//the ui did not change
	}

	
};

void TInputManager::ProcessMessageList(void)
{
	ScopedLock Al(Lock);

	TMouseEvent* Event=0;
	TKeyEvent* KeyEvent=0;

	bool RetVal=false;

	for(unsigned int i=0;i<InputMessageList.size();i++)
	{
		const TInputMessage &Message=InputMessageList[i];
		switch  (Message.MessageType)
		{
			//mouse->QueueDrag( TMousePos( TInputManager::xPos, TInputManager::yPos ) );
		case WM_MOUSEMOVE:
			{
				PosX=GET_X_LPARAM(Message.Param2);
				PosY=GET_Y_LPARAM(Message.Param2);
				if (DragDetect()&& !Dragging && CaptureUI==0)
				{
					//dragging init
					Dragging=true;
					DragObj=new TDragObject(PosX,PosY);
					Event=new TMouseEvent(EMouseEventType::StartDrag,PosX,PosY,false);
					Event->DragObject=DragObj;
				}else 
				if (Dragging && CaptureUI==0)
				{
					//dragging over
					Event=new TMouseEvent(EMouseEventType::DragOver,PosX,PosY,false);
					Event->DragObject=DragObj;
				} else
				{
					//nothing special
					Event=new TMouseEvent(EMouseEventType::Move,PosX,PosY,false);
					
				}
				if (CaptureUI==0)
					RetVal = TMainUI::GetInst().ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);

				if (Event->DragObject!=0)
				{
					if (Event->DragObject->IsCancelled())
					{
						Dragging=false;
						delete DragObj;
						DragObj=0;
					}
				}
				break;
			}
		case WM_LBUTTONDOWN:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);

				Event=new TMouseEvent(EMouseEventType::LeftDn,PosX,PosY,(TimeStruct.GetGlobalTime()-ButtonTimer[0])<DblClickTime);
		
				DragState.X=PosX;
				DragState.Y=PosY;
				MouseDown=true; //to detect drag
				
				ButtonTimer[0]=TimeStruct.GetGlobalTime();

				if (CaptureUI==0)
					RetVal = TMainUI::GetInst().ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);

				break;
			}
		case WM_LBUTTONUP:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);
				if (Dragging)
				{
					//drop 
					Event=new TMouseEvent(EMouseEventType::DragDrop,PosX,PosY,false);
					Event->DragObject=DragObj;
				} else
				{
					//normal btn up
					Event=new TMouseEvent(EMouseEventType::LeftUp,PosX,PosY,false);
				}
				MouseDown=false;

				if (CaptureUI==0)
					RetVal = TMainUI::GetInst().ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				if (Dragging)
				{
					Dragging=false;
					delete DragObj;
					DragObj=0;
				}
			}
		case WM_RBUTTONDOWN:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);
				Event=new TMouseEvent(EMouseEventType::RightDn,PosX,PosY,(TimeStruct.GetGlobalTime()-ButtonTimer[2])<DblClickTime);
				ButtonTimer[2]=TimeStruct.GetGlobalTime();

				if (CaptureUI==0)
					RetVal = TMainUI::GetInst().ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				break;
			}
		case WM_RBUTTONUP:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);
				Event=new TMouseEvent(EMouseEventType::RightUp,PosX,PosY,false);

				if (CaptureUI==0)
					RetVal = TMainUI::GetInst().ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				break;
			}
		case WM_MBUTTONDOWN:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);
				Event=new TMouseEvent(EMouseEventType::MiddleDn,PosX,PosY,(TimeStruct.GetGlobalTime()-ButtonTimer[1])<DblClickTime);
				
				ButtonTimer[1]=TimeStruct.GetGlobalTime();

				if (CaptureUI==0)
					RetVal = TMainUI::GetInst().ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				break;
			}
		case WM_MBUTTONUP:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);
				Event=new TMouseEvent(EMouseEventType::MiddleUp,PosX,PosY,false);

				if (CaptureUI==0)
					RetVal = TMainUI::GetInst().ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				break;
			}
		case WM_MOUSEWHEEL:
			{
				//wheel delta is the base unit for mouse wheel messages
				const int Z=GET_WHEEL_DELTA_WPARAM(Message.Param1)/WHEEL_DELTA; 
				if (Z>0)
				{
					Event=new TMouseEvent(EMouseEventType::WheelUp,PosX,PosY,false);
				}
				else
				{
					Event=new TMouseEvent(EMouseEventType::WheelDn,PosX,PosY,false);
				}
				if (CaptureUI==0)
					RetVal = TMainUI::GetInst().ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				break;
			};
		case WM_KEYDOWN:
			{
				switch (Message.Param1)
				{
				case VK_CONTROL:
					{
						CtrlState=1;
						break;
					}
				case VK_SHIFT:
					{
						ShiftState=1;
						break;
					}
					/*		case VK_UP: case VK_DOWN: case VK_LEFT:    case VK_RIGHT: case VK_ESCAPE:
					{

					switch( wParam )
					{
					case VK_ESCAPE:
					//TMainUI::GetInst().KbHit( wParam );
					break;
					default:
					//boKeyProcess = !TMainUI::GetInst().KbHit( 0, wParam );
					break;
					}*/
				}

				/*TKey vKey;
				vKey.SetKey( Message.Param1,(bool)GetCtrlState(),(bool)GetShiftState() );
				// If that's a valid vkey.
				if( vKey.GetKey() != 0 )
				{
					if( !MacroHdl.CallMacro( vKey ) && !MacroUI::GetInstance()->CallMacro( vKey ) )
					{
						TMainUI::GetInst().KeyInput( Key );
					}
				}*/

				KeyEvent=new TKeyEvent(EKeyEventType::KeyDn,Message.Param1 & 0xFF);
				RetVal = TMainUI::GetInst().ProcessEvent(KeyEvent);
				break;
			}
		case WM_KEYUP:
			{
				switch (Message.Param1)
				{
				case VK_CONTROL:
					{
						CtrlState=0;
						break;
					}
				case VK_SHIFT:
					{
						ShiftState=0;
						break;
					}
				}

				KeyEvent=new TKeyEvent(EKeyEventType::KeyUp,Message.Param1 & 0xFF);
				RetVal = TMainUI::GetInst().ProcessEvent(KeyEvent);
				break;
			}
		case WM_CHAR:
			{
				switch (Message.Param1)
				{
				case VK_TAB:
					{
						//no tab key
						break;
					}
				}
				KeyEvent=new TKeyEvent(EKeyEventType::KeyChar,Message.Param1 & 0xFF);
				RetVal = TMainUI::GetInst().ProcessEvent(KeyEvent);
/*
				if ( (GetCtrlState()==0) || ( isprint( Message.Param1 ) || ( Message.Param1 > 128 && Message.Param1 < 256 ) ) ) 
				{
					if (Global.InGame) 
					{
						if( TMainUI::GetInst().TextInput(Message.Param1 ) )
						{
							break;
						}
						ChatterUI::GetInstance()->TextInput( Message.Param1 );
					} 
				}*/
				break;
			}
		}
		if (Event!=0)
		{
			delete Event;
			Event=0;
		}
		if (KeyEvent!=0)
		{
			delete KeyEvent;
			KeyEvent=0;
		}
	}
	InputMessageList.clear();
};

void TInputManager::Initialize(void)
{

};

void TInputManager::SetPosition(const int NewPosX,const int NewPosY) 
{
	PosX = NewPosX;
	PosY = NewPosY;
}

bool TInputManager::LockNextEvent(unsigned long dwEvent, LPVOID lpCustomPtr, unsigned int (* CallBack)(LPVOID), int i) 
{
	// Refuse mouse event if the mouse is already locked.
	if( LockEvent[i] != 0 )
	{
		return false;
	}

	LockEventCallBack[i] = CallBack;
	LockEvent[i] = dwEvent;
	LockCustomPtr[i] = lpCustomPtr;

	return true;
}

extern bool inAttack;



TMouseCursor::TMouseCursor(void) 
{
	ActualCursor = 9+NbCursors;
	CustomCursor = 0;
	XCor = 0;
	YCor = 0;
}

TMouseCursor::~TMouseCursor(void) 
{
}

void TMouseCursor::SetCustomCursor(TSkinRenderer *NewCursor, int x, int y, bool corr ) 
{
	if (CustomCursor)
	{
		delete CustomCursor;
	}

	CustomCursor = NewCursor;
	XCor = x;
	YCor = y; 
	Correction = corr;
}

void TMouseCursor::ClearCustomCursor(void)
{
	if (CustomCursor)
	{
		delete CustomCursor;
	}
	CustomCursor=0;
	XCor=YCor=0;
	Correction=false;
}


void TMouseCursor::DrawCursor(const float DeltaTime,const int x,const int y) 
{
	int xPos = x;
	int yPos = y;
	int xGridPixel = 32;
	int yGridPixel = 16;

	// If a forced cursor or a custom cursor was specified.
	if (ActualCursor || CustomCursor) 
	{
		// Determine the exact forced cursor to use.
		if ((ActualCursor != 9+NbCursors) && (CustomCursor==0)) 
		{
			//CM simple mouse cursor management
			float Angle=ComputeAngle((float)((x+16)-Global.ScreenWidthDiv2),(float)((y-16)-Global.ScreenHeightDiv2));
			ActualCursor=NbCursors+Angle2Dir(RadToDeg(Angle));
		}


		// Display either the custom vsf cursor or the forced cursor.
		if (CustomCursor) 
		{
			CustomCursor->Render(DeltaTime,(float)(x+XCor), (float)(y+YCor), 0.0f,EItemState::Inventory,0,false);
		} 
		else 
		{
			Cursors[ActualCursor]->AdvanceTime(DeltaTime);
			Cursors[ActualCursor]->DrawSprite( x + XCor, y+YCor, FX_NOCORRECTION );
		}
		return;
	} 
	else 
	{
		Cursors[0]->AdvanceTime(DeltaTime);
		Cursors[0]->DrawSprite( x + XCor, y+YCor, FX_NOCORRECTION );
	}
}

void TMouseCursor::Initialize(void)
{
	AttackCursorIcon.LoadSprite("staticattackcursor");

	MouseCursor1.LoadSprite("64kinterfacecursor");
	MouseCursor2.LoadSprite("attackcursor00");
	MouseCursor3.LoadSprite("64kcursorbow-a");

	MouseCursor5.LoadSprite("north cursor");
	MouseCursor6.LoadSprite("north-east cursor");
	MouseCursor7.LoadSprite("east cursor");
	MouseCursor8.LoadSprite("south-east cursor");
	MouseCursor9.LoadSprite("south cursor");
	MouseCursor10.LoadSprite("south-west cursor");
	MouseCursor11.LoadSprite("west cursor");
	MouseCursor12.LoadSprite("north-west cursor");

	MouseCursor16.LoadSprite("takecursor00");
	MouseCursor17.LoadSprite("talkcursor00");
	MouseCursor18.LoadSprite("doorcursor00");

	Cursors[0] = &MouseCursor1;
	Cursors[1] = &MouseCursor2;
	Cursors[2] = &MouseCursor3;
	Cursors[3] = &MouseCursor1;
	Cursors[4] = &MouseCursor1;
	Cursors[5] = &MouseCursor1;
	Cursors[6] = &MouseCursor1;
	Cursors[7] = &MouseCursor5;
	Cursors[8] = &MouseCursor6;
	Cursors[9] = &MouseCursor7;
	Cursors[10] = &MouseCursor8;
	Cursors[11] = &MouseCursor9;
	Cursors[12] = &MouseCursor10;
	Cursors[13] = &MouseCursor11;
	Cursors[14] = &MouseCursor12;
	Cursors[15] = &MouseCursor1;
	Cursors[16] = &MouseCursor16;
	Cursors[17] = &MouseCursor17;
	Cursors[18] = &MouseCursor18;
}