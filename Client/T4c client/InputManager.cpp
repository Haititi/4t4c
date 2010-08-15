#pragma warning( disable : 4291 )

#include <stdio.h>
#include "pch.h"
#include <windows.h>
#include <windowsx.h>
#include "Debug.h"
#include "InputManager.h"
#include "GfxEngine.h" 
#include "SystemInfo.h"

#include "Netcore.h"
#include "Localizer.h"
#include "App.h"
#include "Global.h"
#include "NewInterface/RootBoxUI.h"
#include "GameUtils.h"
#include "Config.h"
#include "TGameObject.h"
#include "macrohandler.h"
#include "GameTime.h"

TInputManager Input;
TMouseCursor MouseCursor;

extern const int NbCursors;
extern TAnimSprite *lpCursor[19];
extern unsigned long ActualCursor;

const float DblClickTime=0.2f;
const int DragDist=16;

//
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

TAnimSprite *lpCursor[19];


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
	InitializeCriticalSection(&InputCrit);
}


TInputManager::~TInputManager( void )
{
	DeleteCriticalSection(&InputCrit);
}

bool TInputManager::DragDetect(void)
{
	return MouseDown && (abs((DragState.X-PosX)*(DragState.Y-PosY))>DragDist);
};

void TInputManager::StoreMessage(const unsigned int MessageType,const unsigned int Param1,const unsigned int Param2)
{
	EnterCriticalSection(&InputCrit);

	TInputMessage Msg;
	Msg.MessageType=MessageType;
	Msg.Param1=Param1;
	Msg.Param2=Param2;
	Msg.TimeStamp=TimeStruct.GetGlobalTime();

	InputMessageList.push_back(Msg);

	LeaveCriticalSection(&InputCrit);
};

void TInputManager::SetOverUI(TGameUI* GameUI)
{
	if (GameUI!=OverUI)
	{
		if (OverUI!=0)
		{
			TMouseEvent Event(EvtMouse_Leave,PosX,PosY,false);
			//send a mouseLeave
			OverUI->ProcessEvent(&Event);
		}

		OverUI=GameUI;

		if (OverUI!=0)
		{
			TMouseEvent Event(EvtMouse_Enter,PosX,PosY,false);
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
	EnterCriticalSection(&InputCrit);
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
					Event=new TMouseEvent(EvtMouse_StartDrag,PosX,PosY,false);
					Event->DragObject=DragObj;
				}else 
				if (Dragging && CaptureUI==0)
				{
					//dragging over
					Event=new TMouseEvent(EvtMouse_DragOver,PosX,PosY,false);
					Event->DragObject=DragObj;
				} else
				{
					//nothing special
					Event=new TMouseEvent(EvtMouse_Move,PosX,PosY,false);
					
				}
				if (CaptureUI==0)
					RetVal = RootBoxUI::GetInstance()->ProcessEvent(Event);
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

				Event=new TMouseEvent(EvtMouse_LeftDn,PosX,PosY,(TimeStruct.GetGlobalTime()-ButtonTimer[0])<DblClickTime);
		
				DragState.X=PosX;
				DragState.Y=PosY;
				MouseDown=true; //to detect drag
				
				ButtonTimer[0]=TimeStruct.GetGlobalTime();

				if (CaptureUI==0)
					RetVal = RootBoxUI::GetInstance()->ProcessEvent(Event);
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
					Event=new TMouseEvent(EvtMouse_DragDrop,PosX,PosY,false);
					Event->DragObject=DragObj;
				} else
				{
					//normal btn up
					Event=new TMouseEvent(EvtMouse_LeftUp,PosX,PosY,false);
				}
				MouseDown=false;

				if (CaptureUI==0)
					RetVal = RootBoxUI::GetInstance()->ProcessEvent(Event);
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
				Event=new TMouseEvent(EvtMouse_RightDn,PosX,PosY,(TimeStruct.GetGlobalTime()-ButtonTimer[2])<DblClickTime);
				ButtonTimer[2]=TimeStruct.GetGlobalTime();

				if (CaptureUI==0)
					RetVal = RootBoxUI::GetInstance()->ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				break;
			}
		case WM_RBUTTONUP:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);
				Event=new TMouseEvent(EvtMouse_RightUp,PosX,PosY,false);

				if (CaptureUI==0)
					RetVal = RootBoxUI::GetInstance()->ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				break;
			}
		case WM_MBUTTONDOWN:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);
				Event=new TMouseEvent(EvtMouse_MiddleDn,PosX,PosY,(TimeStruct.GetGlobalTime()-ButtonTimer[1])<DblClickTime);
				
				ButtonTimer[1]=TimeStruct.GetGlobalTime();

				if (CaptureUI==0)
					RetVal = RootBoxUI::GetInstance()->ProcessEvent(Event);
				else 
					RetVal = CaptureUI->ProcessEvent(Event);
				break;
			}
		case WM_MBUTTONUP:
			{
				PosX=(short)(Message.Param2 & 0xFFFF);
				PosY=(short)(Message.Param2 >> 16);
				Event=new TMouseEvent(EvtMouse_MiddleUp,PosX,PosY,false);

				if (CaptureUI==0)
					RetVal = RootBoxUI::GetInstance()->ProcessEvent(Event);
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
					Event=new TMouseEvent(EvtMouse_WheelUp,PosX,PosY,false);
				}
				else
				{
					Event=new TMouseEvent(EvtMouse_WheelDn,PosX,PosY,false);
				}
				if (CaptureUI==0)
					RetVal = RootBoxUI::GetInstance()->ProcessEvent(Event);
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
					//RootBoxUI::GetInstance()->KbHit( wParam );
					break;
					default:
					//boKeyProcess = !RootBoxUI::GetInstance()->KbHit( 0, wParam );
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
						RootBoxUI::GetInstance()->KeyInput( Key );
					}
				}*/

				KeyEvent=new TKeyEvent(EvtKey_KeyDn,Message.Param1 & 0xFF);
				RetVal = RootBoxUI::GetInstance()->ProcessEvent(KeyEvent);
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

				KeyEvent=new TKeyEvent(EvtKey_KeyUp,Message.Param1 & 0xFF);
				RetVal = RootBoxUI::GetInstance()->ProcessEvent(KeyEvent);
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
				KeyEvent=new TKeyEvent(EvtKey_Char,Message.Param1 & 0xFF);
				RetVal = RootBoxUI::GetInstance()->ProcessEvent(KeyEvent);
/*
				if ( (GetCtrlState()==0) || ( isprint( Message.Param1 ) || ( Message.Param1 > 128 && Message.Param1 < 256 ) ) ) 
				{
					if (Global.InGame) 
					{
						if( RootBoxUI::GetInstance()->TextInput(Message.Param1 ) )
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

	LeaveCriticalSection(&InputCrit);
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
			CustomCursor->Render(DeltaTime,(float)(x+XCor), (float)(y+YCor), 0.0f,TItemState_Inventory,0,false);
		} 
		else 
		{
			lpCursor[ActualCursor]->AdvanceTime(DeltaTime);
			lpCursor[ActualCursor]->DrawSprite( x + XCor, y+YCor, FX_NOCORRECTION );
		}
		return;
	} 
	else 
	{
		lpCursor[0]->AdvanceTime(DeltaTime);
		lpCursor[0]->DrawSprite( x + XCor, y+YCor, FX_NOCORRECTION );
		// Release the cursor looking for the new UI ownership.

	/*	if( mouseOwned )
		{
			// Only draw the hand icon when in the new interface
			lpCursor[0]->AdvanceTime(DeltaTime);
			lpCursor[0]->DrawSprite( x + iXCor, y+iYCor, FX_NOCORRECTION );            
		}
		else*/
		{
			// Otherwise check what type of unit is at the mouse position
			// (grid coordinates) and show the associated cursor.

		//TODO move that in mouseaction
			//the mouseaction will change the cursor
		/*	if (Grid(((xPos+48)/xGridPixel),(yPos-8)/yGridPixel)) 
			{
				int dwType = Grid(((xPos+48)/xGridPixel),(yPos-8)/yGridPixel); 
				if(dwType == 2 && !Player.GetUseRangedWeapon())
					dwType--;
				else if(dwType ==3) //item
					dwType = 17;     // Bulle pour parler...
				else if(dwType ==4) //Door / coffre
					dwType = 16;     // Door Cursor
				else if(dwType ==5) //item
					dwType = 16;     // take item cursor...
				lpCursor[dwType]->AdvanceTime(DeltaTime);
				lpCursor[dwType]->DrawSprite( x + iXCor, y+iYCor, FX_NOCORRECTION);
			} 
			else 
			{
				const unsigned long CursorNum=Grid(((xPos+48)/xGridPixel),(yPos-8)/yGridPixel);
				lpCursor[CursorNum]->AdvanceTime(DeltaTime);
				lpCursor[CursorNum]->DrawSprite( x + iXCor, y+iYCor, FX_NOCORRECTION );
			}*/
		}
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

	lpCursor[0] = &MouseCursor1;
	lpCursor[1] = &MouseCursor2;
	lpCursor[2] = &MouseCursor3;
	lpCursor[3] = &MouseCursor1;
	lpCursor[4] = &MouseCursor1;
	lpCursor[5] = &MouseCursor1;
	lpCursor[6] = &MouseCursor1;
	lpCursor[7] = &MouseCursor5;
	lpCursor[8] = &MouseCursor6;
	lpCursor[9] = &MouseCursor7;
	lpCursor[10] = &MouseCursor8;
	lpCursor[11] = &MouseCursor9;
	lpCursor[12] = &MouseCursor10;
	lpCursor[13] = &MouseCursor11;
	lpCursor[14] = &MouseCursor12;
	lpCursor[15] = &MouseCursor1;
	lpCursor[16] = &MouseCursor16;
	lpCursor[17] = &MouseCursor17;
	lpCursor[18] = &MouseCursor18;
}