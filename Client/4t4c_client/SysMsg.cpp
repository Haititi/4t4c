#include "SysMsg.h"
#include "Gametime.h"

extern TD3dFont* TalkFont;

TSysMsgList SysMsgList;

TSysMsg::TSysMsg(void) : TimeStamp( 0.0f ), Text( 0 )
{
	Text = TalkFont->CreateTextObject();
	Text->SetBackGround(false);
	Text->SetFx(TextFX_Outline);
}

TSysMsg::~TSysMsg(void)
{
    delete Text;
}

void TSysMsg::SetText( const char *Txt, const unsigned long BaseColor,const float Time,const bool MultiLine)
{
	TimeStamp=Time;
	Color=BaseColor;
	
	Text->SetColor(BaseColor);
	Text->SetMaxSize(GfxCore.ScreenWidth(),GfxCore.ScreenHeight());
	Text->SetText(Txt);
}

void TSysMsg::Draw( const int x,const int y ,const unsigned long Alpha)
{
    if( Text != 0 )
	{
		Text->SetColor((Color & 0x00FFFFFF) | (Alpha<<24));
        Text->DrawAt( x, y );
    }   
}

unsigned int TSysMsg::GetHeight()
{
    if( Text == 0 )
	{
        return 0;
    }
    return Text->GetHeight();
}

TSysMsgList::TSysMsgList(void)
{
	
};

TSysMsgList::~TSysMsgList(void)
{
	
};

void TSysMsgList::AddMessage(const char *Text, unsigned long Color, float Time, bool AllowNewLine)
{
	TSysMsg *Msg = new TSysMsg; 

	Msg->SetText( Text, Color, Time, AllowNewLine);

	{
		ScopedLock Al(Lock);
		MsgList.push_back(Msg); 
	}

};

	
void TSysMsgList::Render(float DeltaTime)
{
	ScopedLock Al(Lock);
	//erase the older message in the list
	std::list<TSysMsg*>::iterator MsgListIt=MsgList.begin();
	if (MsgListIt!=MsgList.end())
	{
		if ((TimeStruct.GetGlobalTime()-(*MsgListIt)->GetTimestamp()) > 6.0f)
		{
			delete (*MsgListIt);
			MsgList.erase(MsgListIt);
		}
	}

	//Draw the list in reverse order
	std::list<TSysMsg*>::reverse_iterator MsgListRIt=MsgList.rbegin();
	int PositionY = 0;
	while(MsgListRIt!=MsgList.rend()) 
	{
		
		//compute alpha value ,  if time >4 seconds we fade the text away
		const float TimeLeft=6.0f-(TimeStruct.GetGlobalTime()-(*MsgListRIt)->GetTimestamp());
		unsigned long Alpha=0xFF;
		if (TimeLeft<=2.0f)
		{
			Alpha=(unsigned long)(TimeLeft*127.0f);
		}
		(*MsgListRIt)->Draw( 20, GfxCore.ScreenHeight()-45 - PositionY - (*MsgListRIt)->GetHeight(),Alpha );
		PositionY += (*MsgListRIt)->GetHeight();
		MsgListRIt++;
	};
};
