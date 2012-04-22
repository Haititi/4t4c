#ifndef NETCLI_H
#define NETCLI_H

#include "Headers.h"
#include "Lock.h"
#include "Datagram.h"
#include "Packet.h"
#include "../hashpool.h"

//timeout in ms
const unsigned int ClientTimeOut=1000;

class TNetworkCore
{
private:
	HANDLE HdlThreadSend,HdlThreadReceive;
	unsigned int PakAggloWaitTime;
	sockaddr_in BindSockAddr,SendSockAddr;  
	SOCKET TheSocket;

	unsigned long MinPing,AvgPing,MaxPing;
	unsigned long LastReceiveStamp;

	friend long ThreadWrapperReceive(TNetworkCore *Ntw);
	friend long ThreadWrapperSend(TNetworkCore *Ntw);
	bool ThreadReceiveTerminated;
	bool ThreadSendTerminated;
	void ThreadReceive(void);
	void ThreadSend(void);

	void UseInputBuffer(TDatagramBuffer* DatagramBuf);

	CriticalSection LockInPack;
	std::vector<TPacket*> InputPackList;
	void StoreInputPacket(TPacket* Packet);

	//we probably don't need sync here , only the receive thread will access it
	THashPool FragmentList; //hashlist of TDatagram


	CriticalSection LockWaitAck;
	THashPool WaitingForAckPool; //Hashlist of TDatagramBuf

	//Datagram being prepared
	CriticalSection LockPrepareList;
	std::vector<TDataGram*> PreparationList;


	//void SendDatagram(TDataGram* Datagram);//send datagram Buffers and store to ack list (if needed)
	void Acknowledge(const unsigned short UniqueID,const unsigned short FragmentNum,const unsigned long TimeStamp);

	unsigned int WaitTimeoutTime;

public:
	TNetworkCore(void);
	~TNetworkCore(void);

	//setup
	bool Setup(std::string& Ip,unsigned short Port);
	void SetWaitTimeForPakAgglomeration(unsigned int NewTimeMs){PakAggloWaitTime=NewTimeMs;};

	//connection
	void Connect(void);  //client or server side method to start the network
	bool IsConnectionAlive(void);//connection died?
	bool SlowTraffic(void); //connection slow? (high answer time)
	unsigned long AveragePing(void); //return average ping time in millisecond
	void Terminate(void); //Blocking function 

	//sending
	void SendPacket(TPacket* Packet); 

	//receiving
	TPacket* GetNextPacket(void); //return a packet that was received , null otherwise
};

long ThreadWrapperReceive(TNetworkCore *Cli);
long ThreadWrapperSend(TNetworkCore *Cli);

extern TNetworkCore NetCore;
#endif