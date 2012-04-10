#include "Packet.h"
#include "PacketTypes.h"

const unsigned int PackGranul=64;
#define VoidAddr(Obj) (unsigned int*)((unsigned int)Obj)

TPacket::TPacket(void):BufferSize(0)
{
	Buffer=malloc(PackGranul);
	BufferCapacity=PackGranul;
	Position=Buffer;
};

TPacket::TPacket(const unsigned short NewPakId):Position(0),BufferSize(0),PakId(NewPakId)
{
	Buffer=malloc(PackGranul);
	BufferCapacity=PackGranul;
	Position=Buffer;
};

TPacket::TPacket(const void* const CpyFromBuf,const unsigned int CpyFromSize):BufferSize(CpyFromSize),Position(0)
{
	Buffer=malloc(PackGranul);
	BufferCapacity=PackGranul;
	Position=Buffer;

	WriteData(CpyFromBuf,CpyFromSize);

	//reset pos
	Position=Buffer;
};

TPacket::~TPacket(void)
{
	if (Buffer)
		free(Buffer);
};


void TPacket::Grow(const unsigned int Quantity)
{
	unsigned int addsize,intpos;
	intpos=(unsigned int) Position-(unsigned int) Buffer;
    if (Quantity<PackGranul) 
		addsize=PackGranul; else 
		addsize=(PackGranul-(Quantity%PackGranul))+Quantity;
	BufferCapacity+=addsize;
	Buffer=realloc(Buffer, BufferCapacity);
	Position=(unsigned int*)((unsigned int) Buffer+intpos); 
};

void TPacket::WriteData(const void * const DataPtr,const unsigned int Size)
{
	if (Size!=0)
	{
		BufferSize+=Size;
		if (BufferSize>BufferCapacity)
			Grow(Size);
		
		memcpy(Position,DataPtr,Size);

		Position=VoidAddr(Position+Size);
	}
};

void TPacket::ReadData(void * const DataPtr,const unsigned int Size)
{
	if (Size!=0)
	{
		if((GetIntPosition()+ Size)>BufferSize)
		{
			*((char*)DataPtr)=0; //in case
			return;
		}
			
		memcpy(DataPtr,Position,Size);

		Position=VoidAddr(Position+Size);
	}
};

void TPacket::WriteChar(const char Value)
{
	WriteUChar((unsigned char) Value);
}

void TPacket::WriteShort(const short Value)
{
	WriteUShort((unsigned short) Value);
}

void TPacket::WriteLong(const long Value)
{
	WriteULong((unsigned long) Value);
}

void TPacket::WriteUChar(const unsigned char Value)
{
	BufferSize+=sizeof(unsigned char);
	if (BufferSize>BufferCapacity)
		Grow(sizeof(unsigned char));

	(*(unsigned char*)Position)=Value;
	Position=VoidAddr(Position+sizeof(unsigned char));
}
	
void TPacket::WriteUShort(const unsigned short Value)
{
	BufferSize+=sizeof(unsigned short);
	if (BufferSize>BufferCapacity)
		Grow(sizeof(unsigned short));

	(*(unsigned short*)Position)=Value;
	Position=VoidAddr(Position+sizeof(unsigned short));
}
	

void TPacket::WriteULong(const unsigned long Value)
{
	BufferSize+=sizeof(unsigned long);
	if (BufferSize>BufferCapacity)
		Grow(sizeof(unsigned long));

	(*(unsigned long*)Position)=Value;
	Position=VoidAddr(Position+sizeof(unsigned long));
}

void TPacket::WriteShortString(const char*const Str)
{
	if (Str!=0)
	{
		const unsigned short Len=strlen(Str);
		WriteUShort(Len);
		WriteData(Str,Len);
	} else
	{
		WriteUShort(0);
	}
};

char* TPacket::ReadShortString()
{
	const unsigned short Len=ReadUShort();
	char* Result;
	if (Len!=0)
	{
		Result=new char[Len+1];
		ReadData(Result,Len);	
	}else
	{
		Result=new char[1];
	}
	Result[Len]=0;
	
	return Result;
};

signed char TPacket::ReadChar(void)
{
	return (signed char) ReadUChar();
};
	
signed short TPacket::ReadShort(void)
{
	return (signed short) ReadUShort();
};
	
signed long TPacket::ReadLong(void)
{
	return (signed long) ReadULong();
};

unsigned char TPacket::ReadUChar(void)
{
	unsigned char Result  = 0;
	if((GetIntPosition()+ sizeof( unsigned char ))<BufferSize)
		Result = *((unsigned char*)Position);

	Position=VoidAddr(Position+sizeof(unsigned char));
	return Result;
};
	
unsigned short TPacket::ReadUShort(void)
{
	unsigned short Result = 0;
	if((GetIntPosition()+ sizeof( unsigned short ))<BufferSize)
		Result = *((unsigned short*)Position);

	Position=VoidAddr(Position+sizeof(unsigned short));
	return Result;
};
	
unsigned long TPacket::ReadULong(void)
{
	unsigned long Result = 0;
	if((GetIntPosition()+ sizeof( unsigned long ))<BufferSize)
		Result = *((unsigned long*)Position);

	Position=VoidAddr(Position+sizeof(unsigned long));
	return Result;
};

__int64 TPacket::ReadInt64(void)
{
	__int64 Result=0;
	if((GetIntPosition()+ sizeof( __int64 ))<BufferSize)
		Result = *((__int64*)Position);

	Position=VoidAddr(Position+sizeof(__int64));
	return Result;
};

void TPacket::CopyShortString(char * Str,int DestSize)
{
	unsigned short Len=ReadUShort();
	Str[0]=0;
	if (Len!=0)
	{
		if((GetIntPosition()+ Len)<BufferSize)
		{
			if (Len>DestSize)
				Len=DestSize-1;
			for (int i=0;i<Len;i++)
				Str[i]=*(((char*)Position)+i);
			Str[Len]=0;
		}
		Position=VoidAddr(Position+Len);
	}
};

void TPacket::Pack_RQ_PlayerMove(ECharDirection::Enum Direction)//1-8
{
	if ((Direction==0) || (Direction>8))
		Direction=ECharDirection::Up;
	SetPakId(Direction);
};

void TPacket::Pack_RQ_ViewBackpack(const short Unknown)//18
{
	SetPakId(RQ_ViewBackpack);
	WriteUShort(Unknown);
};

void TPacket::Pack_RQ_ExitGame(void)//20
{
	SetPakId(RQ_ExitGame);
}

void TPacket::Pack_RQ_DeleteCharacter(const char* CharName)
{
	SetPakId(RQ_DeletePlayer);
	WriteShortString(CharName);
};

void TPacket::Pack_RQ_GetCharList(void)
{
	SetPakId(RQ_GetCharacterList);
};

void TPacket::Pack_RQ_FromPreInGameToInGame(void)
{
	SetPakId(RQ_FromPreInGameToInGame);
};

void TPacket::Pack_RQ_UseSkill(const short SkillId,const short TargetX,const short TargetY,const long TargetId)
{
	SetPakId(RQ_UseSkill);
	WriteUShort(SkillId);
	WriteUShort(TargetX);
	WriteUShort(TargetY);
	WriteULong( TargetId);
};

void TPacket::Pack_RQ_GetStatus(void)//43
{
	SetPakId(RQ_GetStatus);
};


void TPacket::Pack_RQ_SendStatTrain(const char Str,const char End,const char Agi,const char Wil,const char Wis,const char Int,const char Luk)//58
{
	SetPakId(RQ_SendStatTrain);
	WriteUChar(Str);
	WriteUChar(End);
	WriteUChar(Agi);
	WriteUChar(Wil);
	WriteUChar(Wis);
	WriteUChar(Int);
	WriteUChar(Luk);
};

void TPacket::Pack_RQ_Page(const char* Name,const char* Msg)//29
{
	SetPakId(RQ_Page);
	WriteShortString(Name);
	WriteShortString(Msg);
};

void TPacket::Pack_RQ_BroadcastTextChange(const long Id,const short Offset)
{
	SetPakId(RQ_BroadcastTextChange);
	WriteULong(Id);
	WriteUShort(Offset);
};

void TPacket::Pack_RQ_GetNearItems(void)
{
	SetPakId(RQ_GetNearItems);
};

void TPacket::Pack_RQ_Shout(const char* Name,const char* Msg,const long Color)
{
	SetPakId(RQ_Shout);
	WriteShortString(Name);
	WriteULong(Color);
	WriteShortString(Msg);
};

void TPacket::Pack_RQ_DirectedTalk(const short PosX,const short PosY,const unsigned long Id,const char Direction,const unsigned long Color,const char* Msg)//30
{
	SetPakId(RQ_DirectedTalk);
	WriteUShort(PosX);
	WriteUShort(PosY);
	WriteULong(Id);
	WriteUChar(Direction);
	WriteULong(Color);
	if (Msg)
	{
		WriteShortString(Msg);
	} else
	{
		WriteUShort(0);
	}
};

void TPacket::Pack_RQ_UndirectedTalk(const unsigned long Id,const char Direction,const unsigned long Color,const char* Msg)//27
{
	SetPakId(RQ_IndirectTalk);
	WriteULong(Id);
	WriteUChar(0);
	WriteULong(Color);
	WriteShortString(Msg);
};

void TPacket::Pack_RQ_QueryItemName(const char SearchLocation,const long ItemId)//59
{
	SetPakId(RQ_QueryItemName);
	WriteUChar(SearchLocation);
	WriteULong(ItemId);
};

void TPacket::Pack_RQ_UseObject(const short Posx,const short Posy,const long Id)
{
	SetPakId(RQ_UseObject);
	WriteUShort(Posx);
	WriteUShort(Posy);
	WriteULong(Id);
};

void TPacket::Pack_RQ_GetObject(const short Posx,const short Posy,const long Id)
{
	SetPakId(RQ_GetObject);
	WriteUShort(Posx);
	WriteUShort(Posy);
	WriteULong(Id);
};

void TPacket::Pack_RQ_Attack(const short Posx,const short Posy,const long Id)
{
	SetPakId(RQ_Attack);
	WriteUShort(Posx);
	WriteUShort(Posy);
	WriteULong(Id);
};

void TPacket::Pack_RQ_EnterChannel(const char* Channel,const char *Password)//48
{
	SetPakId(RQ_EnterChannel);
	WriteShortString(Channel);
	if (Password!=NULL)
	{
		WriteShortString(Password);
	} else
	{
		WriteUShort(0);
	}
};

void TPacket::Pack_RQ_SendChatterChannelMessage(const char* Channel,const char* Msg)
{
	SetPakId(RQ_SendChatterChannelMessage);
	WriteShortString(Channel);
	WriteShortString(Msg);
};

void TPacket::Pack_RQ_GetChannelUsersList(const char* Channel)//50
{
	SetPakId(RQ_GetChannelUsersList);
	WriteShortString(Channel);
};

void TPacket::Pack_RQ_LeaveChannel(const char* Channel)//74
{
	SetPakId(RQ_LeaveChannel);
	WriteShortString(Channel);
};

void TPacket::Pack_RQ_GetPublicChannelList(void)//75
{
	SetPakId(RQ_GetPublicChannelList);
};

void TPacket::Pack_RQ_ToggleChannelListening(const char* Channel,const char State)//86
{
	SetPakId(RQ_ToggleChannelListening);
	WriteShortString(Channel);
	WriteUChar(State);
};

void TPacket::Pack_RQ_TogglePage(const char State)
{
	SetPakId(RQ_TogglePage);
	WriteUChar(State);
};

void TPacket::Pack_RQ_ChestToBackpack(const long ItemID,const long Quantity)
{
	SetPakId(RQ_ChestToBackpack);
	WriteULong(ItemID);
	WriteULong(Quantity);
};

void TPacket::Pack_RQ_ChestFromBackpack(const long ItemID,const long Quantity)
{
	SetPakId(RQ_ChestFromBackpack);
	WriteULong(ItemID);
	WriteULong(Quantity);
};


void TPacket::Pack_RQ_GroupJoin(void)//79
{
	SetPakId(RQ_GroupJoin);
};

void TPacket::Pack_RQ_GroupInvite(const long PlayerId,const short Posx,const short Posy)//78
{
	SetPakId(RQ_GroupInvite);
	WriteULong(PlayerId);
	WriteUShort(Posx);
	WriteUShort(Posy);
};

void TPacket::Pack_RQ_GroupLeave(void)//80
{
	SetPakId(RQ_GroupLeave);
};

void TPacket::Pack_RQ_GroupKick(const long PlayerId)//81
{
	SetPakId(RQ_GroupKick);
	WriteULong(PlayerId);
};

void TPacket::Pack_RQ_GroupToggleAutoSplit(const char State)//88
{
	SetPakId(RQ_GroupToggleAutoSplit);
	WriteUChar(State);
};

void TPacket::Pack_RQ_PutPlayerInGame(const char *CharName)
{
	SetPakId(RQ_PutPlayerInGame); //PutPlayerIngame
	WriteShortString(CharName);
};

void TPacket::Pack_RQ_GetPlayerPos()//9
{
	SetPakId(RQ_GetPlayerPos);
};


void TPacket::Pack_RQ_SendPeriphericObjects(const char Direction,const short PosX,const short PosY)//16
{
	SetPakId(RQ_SendPeriphericObjects);
	WriteUChar(Direction);
	WriteUShort(PosX);
	WriteUShort(PosY);
};

void TPacket::Pack_RQ_RegisterAccount(const char* Login,const char* Password,const short Version)//14
{
	SetPakId(RQ_RegisterAccount);
	WriteShortString(Login);
	WriteShortString(Password);
	WriteUShort(Version);
	WriteUShort(0);
};


void TPacket::Pack_RQ_GuildGetMembers(void)//123
{
	SetPakId(RQ_GuildGetMembers);
}

void TPacket::Pack_RQ_GuildInvite(const short Posx,const short Posy,const long Id)//124
{
	SetPakId(RQ_GuildInvite);
	WriteULong(Id);
	WriteUShort(Posx);
	WriteUShort(Posy);   
};

void TPacket::Pack_RQ_GuildKick(const long Id)//125
{
	SetPakId(RQ_GuildKick);
	WriteULong(Id);
};

void TPacket::Pack_RQ_GuildLeave(void)//126
{
	SetPakId(RQ_GuildLeave);
};

void TPacket::Pack_RQ_GuildInviteAnswer(const long Id,const bool Answer)//129
{
	SetPakId(RQ_GuildInviteAnswer);
	WriteULong( Id);
	WriteUChar( Answer); // Decline
};

void TPacket::Pack_RQ_GetUnitName(const unsigned long Id,const unsigned short PosX,const unsigned short PosY)//35
{
	SetPakId(RQ_GetUnitName);
	WriteULong(Id);
	WriteUShort(PosX);
	WriteUShort(PosY);   
};

void TPacket::Pack_RQ_ServerPatchInfo()//91
{
	SetPakId(RQ_ServerPatchInfo);
	//WriteUShort( RQ_ServerPatchInfo);
};

void TPacket::Pack_RQ_BreakConversation(const short PosX,const short PosY,const unsigned long Id)//36
{
	SetPakId(RQ_BreakConversation);
	WriteULong(Id);
	WriteUShort(PosX);
	WriteUShort(PosY);   
};

void TPacket::Pack_RQ_QueryUnitExist(const unsigned long Id,const short PosX,const short PosY)//71
{
	SetPakId(RQ_QueryUnitExist);
	WriteULong(Id);
	WriteUShort(PosX);
	WriteUShort(PosY);   
};

void TPacket::Pack_RQ_UseItemBySkin(const short SkinId)//72
{
	SetPakId(RQ_UseItemBySkinId);
	WriteULong(SkinId);
};

void TPacket::Pack_RQ_DropObject(const short PosX,const short PosY,const unsigned long Id,const unsigned long Qty)//12
{
	SetPakId(RQ_DropObject);
	WriteUShort(PosX);
	WriteUShort(PosY);   
	WriteULong(Id);
	WriteULong(Qty);
};

void TPacket::Pack_RQ_CastSpell(const short SpellId,const short PosX,const short PosY,const unsigned long TargetId)//32
{
	SetPakId(RQ_CastSpell);
	WriteUShort(SpellId);
	WriteUShort(PosX);
	WriteUShort(PosY);
	WriteULong(TargetId);
};

void TPacket::Pack_RQ_TradeSetStatus(const short Status)//117
{
	SetPakId(RQ_TradeSetStatus);
	WriteUShort(Status);
};

void TPacket::Pack_RQ_TradeCancel(void)//118
{
	SetPakId(RQ_TradeCancel);
};

void TPacket::Pack_RQ_TradeClear(void)//119
{
	SetPakId(RQ_TradeClear);
};

void TPacket::Pack_RQ_TradeInvite(const unsigned long TargetId,const short TargetX,const short TargetY)//116
{
	SetPakId(RQ_TradeInvite);
	WriteULong(TargetId);
	WriteUShort(TargetX);
	WriteUShort(TargetY);
};

void TPacket::Pack_RQ_SendTrainingSkill(const short NpcX,const short NpcY,const unsigned long NpcId,const short SkillCount)//40
{
	SetPakId(RQ_SendTrainSkillList);
	WriteUShort(NpcX);
	WriteUShort(NpcY);
	WriteULong(NpcId);
	WriteUShort(SkillCount);
};

void TPacket::Pack_RQ_SendTrainingSkillAdd(const short SkillId,const short Qty)
{
	WriteUShort(SkillId);
	WriteUShort(Qty);
};

void TPacket::Pack_RQ_SendLearningSkill(const short NpcX,const short NpcY,const unsigned long NpcId,const short SkillCount)//55
{
	SetPakId(RQ_SendLearningSkillList);
	WriteUShort(NpcX);
	WriteUShort(NpcY);
	WriteULong(NpcId);
	WriteUShort(SkillCount);
};

void TPacket::Pack_RQ_SendLearningSkillAdd(const short SkillId)
{
	WriteUShort(SkillId);
};

void TPacket::Pack_RQ_GetSkillList(void)
{
	SetPakId(RQ_GetSkillList);
};

void TPacket::Pack_RQ_SendBuyItemList(const short Posx,const short Posy,const long NpcId,const short ItemCount)//41
{
	SetPakId(RQ_SendBuyItemList);
	WriteUShort(Posx);
	WriteUShort(Posy);
	WriteULong(NpcId);      
	WriteUShort(ItemCount);
};
//add one item to the pack above
void TPacket::Pack_RQ_SendBuyItemListAddItem(const short ItemId,const short Quantity)//41
{
	WriteUShort(ItemId);
	WriteUShort(Quantity);
};

void TPacket::Pack_RQ_SendSellItemList(const short Posx,const short Posy,const long NpcId,const short ItemCount)//56
{
	SetPakId(RQ_SendSellItemList);
	WriteUShort(Posx);
	WriteUShort(Posy);
	WriteULong(NpcId);      
	WriteUShort(ItemCount);
};

void TPacket::Pack_RQ_SendSellItemListAddItem(const short ItemId,const short Qty)//56
{
	WriteUShort(ItemId);
	WriteUShort(Qty);
};


void TPacket::Pack_RQ_QueryItemInfo(const unsigned long ItemId)
{
	SetPakId( RQ_QueryItemInfo);
	WriteULong( ItemId);
};

void TPacket::Pack_RQ_AuthenticateServerVersion(const unsigned long ClientVer)
{
	SetPakId( RQ_AuthenticateServerVersion);
	WriteULong( ClientVer);
};

void TPacket::Pack_RQ_ViewEquiped(void)//19
{
	SetPakId( RQ_ViewEquiped);
};

void TPacket::Pack_RQ_JunkItems(const unsigned long ItemId,const unsigned long Qty)//85
{
	SetPakId( RQ_JunkItems);
	WriteULong( ItemId);
	WriteULong( Qty);
};

void TPacket::Pack_RQ_RobItem(const unsigned long ItemId)//93
{
	SetPakId( RQ_RobItem);
	WriteULong( ItemId);
}

void TPacket::Pack_RQ_GetSpellList(const char Unknow)//62
{
	SetPakId( RQ_GetSpellList);
	WriteUChar( Unknow);
};

void TPacket::Pack_RQ_TradeFromBackpackToTrade(const unsigned long ItemId,const unsigned long Qty)//112
{
	SetPakId( RQ_TradeFromBackpackToTrade);
	WriteULong( ItemId);
	WriteULong( Qty);
};

void TPacket::Pack_RQ_TradeFromTradeToBackpack(const unsigned long ItemId,const unsigned long Qty)//113
{
	SetPakId( RQ_TradeFromTradeToBackpack);
	WriteULong( ItemId);
	WriteULong( Qty);
};

void TPacket::Pack_RQ_KeepAlive(void)//10
{
	SetPakId( RQ_KeepAlive);
};