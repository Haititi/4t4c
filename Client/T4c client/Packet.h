#ifndef PACKET_H
#define PACKET_H

#include "Headers.h"
//custom serialization class to build network pack

//TODO move that somewhere else......
enum TCharDirection
{
	CharDir_Up=1,
	CharDir_UpRight,
	CharDir_Right,
	CharDir_DownRight,
	CharDir_Down,
	CharDir_DownLeft,
	CharDir_Left,
	CharDir_UpLeft
};

class TPacket
{
private:
	void* Buffer;
	void* Position;
	unsigned int BufferSize,BufferCapacity;

	unsigned short PakId;

	void Grow(const unsigned int Quantity);

	//set as private, nobody really need to access it directly
	void WriteData(const void * const DataPtr,const unsigned int Size);
	void WriteChar(const char Value);
	void WriteShort(const short Value);
	void WriteLong(const long Value);

	void WriteUChar(const unsigned char Value);
	void WriteUShort(const unsigned short Value);
	void WriteULong(const unsigned long Value);

	void WriteShortString(const char*const Str);

	unsigned int GetIntPosition(void){return (unsigned int)Position-(unsigned int)Buffer;};
public:
	TPacket(void);
	TPacket(const unsigned short NewPakId);
	TPacket(const void* const CpyFromBuf,const unsigned int CpyFromSize);
	~TPacket(void);

	//Stream func
	void ReadData(void * const DataPtr,const unsigned int Size);
	//that one allocate memory and return the string
	char* ReadShortString();
	//those 2 copy the string into the provided buffer
	void CopyShortString(char* Str,int DestSize);
	signed char    ReadChar(void);
	signed short   ReadShort(void);
	signed long    ReadLong(void);
	unsigned char  ReadUChar(void);
	unsigned short ReadUShort(void);
	unsigned long  ReadULong(void);
	__int64		   ReadInt64(void);

	void SetPakId(const unsigned short NewPakId){PakId=NewPakId;};
	unsigned short GetPakId(void){return PakId;};

	void* GetBuffer(void){return Buffer;};
	unsigned int GetBufferSize(void){return BufferSize;};

	//pak building functions
	void Pack_RQ_KeepAlive(void);//10

	void Pack_RQ_PlayerMove(TCharDirection Direction); //1->8
	void Pack_RQ_GetPlayerPos(void);//9
	void Pack_RQ_RegisterAccount(const char* Login,const char* Password,const short Version);//14
	void Pack_RQ_DeleteCharacter(const char* CharName);//15
	void Pack_RQ_SendPeriphericObjects(const char Direction,const short PosX,const short PosY);//16
	void Pack_RQ_ViewBackpack(const short Unknown);//18
	void Pack_RQ_ViewEquiped(void);//19
	void Pack_RQ_ExitGame(void);//20
	void Pack_RQ_GetCharList(void);//26

	void Pack_RQ_GetUnitName(const unsigned long Id,const unsigned short PosX,const unsigned short PosY);//35
	
	void Pack_RQ_GetSkillList(void);//39
	void Pack_RQ_SendLearningSkill(const short NpcX,const short NpcY,const unsigned long NpcId,const short SkillCount);//55
	void Pack_RQ_SendLearningSkillAdd(const short SkillId);
	void Pack_RQ_SendTrainingSkill(const short NpcX,const short NpcY,const unsigned long NpcId,const short SkillCount);//40
	void Pack_RQ_SendTrainingSkillAdd(const short SkillId,const short Qty);
	void Pack_RQ_SendBuyItemList(const short Posx,const short Posy,const long NpcId,const short ItemCount);//41
	void Pack_RQ_SendBuyItemListAddItem(const short ItemId,const short Quantity);//41
	void Pack_RQ_SendSellItemList(const short Posx,const short Posy,const long NpcId,const short ItemCount);//56
	void Pack_RQ_SendSellItemListAddItem(const short ItemId,const short Qty);//56
	void Pack_RQ_UseSkill(const short SkillId,const short TargetX,const short TargetY,const long TargetId);//42
	void Pack_RQ_UseItemBySkin(const short SkinId);//72
	void Pack_RQ_GetStatus(void);//43

	void Pack_RQ_PutPlayerInGame(const char *CharName);
	void Pack_RQ_FromPreInGameToInGame(void);//46
	void Pack_RQ_EnterChannel(const char* Channel,const char *Password);//48
	void Pack_RQ_SendChatterChannelMessage(const char* Channel,const char* Msg); //49
	void Pack_RQ_LeaveChannel(const char* Channel);//74
	void Pack_RQ_GetPublicChannelList(void);//75
	void Pack_RQ_GetChannelUsersList(const char* Channel);//50
	void Pack_RQ_ToggleChannelListening(const char* Channel,const char State);//86

	void Pack_RQ_SendStatTrain(const char Str,const char End,const char Agi,const char Wil,const char Wis,const char Int,const char Luk);//58

	void Pack_RQ_Page(const char* Name,const char* Msg);//29
	void Pack_RQ_TogglePage(const char State);//89
	void Pack_RQ_Shout(const char* Name,const char* Msg,const long Color);
	void Pack_RQ_BroadcastTextChange(const long Id,const short Offset);
	void Pack_RQ_DirectedTalk(const short PosX,const short PosY,const unsigned long Id,const char Direction,const unsigned long Color,const char* Msg);//30
	void Pack_RQ_UndirectedTalk(const unsigned long Id,const char Direction,const unsigned long Color,const char* Msg);//27
	void Pack_RQ_BreakConversation(const short PosX,const short PosY,const unsigned long Id);//36

	void Pack_RQ_GetNearItems(void);
	void Pack_RQ_UseObject(const short Posx,const short Posy,const long Id);
	void Pack_RQ_GetObject(const short Posx,const short Posy,const long Id);
	void Pack_RQ_DropObject(const short PosX,const short PosY,const unsigned long Id,const unsigned long Qty);//12
	void Pack_RQ_RobItem(const unsigned long ItemId);//93
	void Pack_RQ_JunkItems(const unsigned long ItemId,const unsigned long Qty);//85
	void Pack_RQ_Attack(const short Posx,const short Posy,const long Id);
	void Pack_RQ_QueryItemName(const char SearchLocation,const long ItemId);//59
	void Pack_RQ_QueryUnitExist(const unsigned long Id,const short PosX,const short PosY);//71
	void Pack_RQ_QueryItemInfo(const unsigned long ItemId);//122
	void Pack_RQ_GetSpellList(const char Unknow);//62
	void Pack_RQ_CastSpell(const short SpellId,const short PosX,const short PosY,const unsigned long TargetId);//32

	void Pack_RQ_ChestToBackpack(const long ItemID,const long Quantity);
	void Pack_RQ_ChestFromBackpack(const long ItemID,const long Quantity);

	void Pack_RQ_TradeInvite(const unsigned long TargetId,const short TargetX,const short TargetY);//116
	void Pack_RQ_TradeSetStatus(const short Status);//117
	void Pack_RQ_TradeCancel(void);//118
	void Pack_RQ_TradeClear(void);//119
	void Pack_RQ_TradeFromBackpackToTrade(const unsigned long ItemId,const unsigned long Qty);//112
	void Pack_RQ_TradeFromTradeToBackpack(const unsigned long ItemId,const unsigned long Qty);//113
	

	void Pack_RQ_GroupInvite(const long PlayerId,const short Posx,const short Posy);//78
	void Pack_RQ_GroupJoin(void);//79
	void Pack_RQ_GroupLeave(void);//80
	void Pack_RQ_GroupKick(const long PlayerId);//81
	void Pack_RQ_GroupToggleAutoSplit(const char State);//88

	void Pack_RQ_ServerPatchInfo();//91
	void Pack_RQ_AuthenticateServerVersion(const unsigned long ClientVer);//99

	void Pack_RQ_GuildGetMembers(void);//123
	void Pack_RQ_GuildInvite(const short Posx,const short Posy,const long Id);//124
	void Pack_RQ_GuildKick(const long Id);//125
	void Pack_RQ_GuildLeave(void);//126
	void Pack_RQ_GuildInviteAnswer(const long Id,const bool Answer);//129
};

#endif