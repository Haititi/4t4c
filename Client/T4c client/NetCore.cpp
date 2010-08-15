#include "NetCore.h"
#include "packettypes.h"
#include "../hash.h"

TNetworkCore NetCore;

inline unsigned long FragHash(const unsigned short UniqueID,const unsigned short FragmentNum)
{
	return (FragmentNum<<16)|UniqueID;
}

int SockLen=sizeof(sockaddr_in);

TNetworkCore::TNetworkCore(void):WaitingForAckPool(109),FragmentList(109)
{
	InitRandHash();

	ThreadSendTerminated=false;
	ThreadReceiveTerminated=false;

	MinPing=100000;
	AvgPing=50;
	MaxPing=0;

	PakAggloWaitTime=5;

	WSADATA wsaData;
	if (WSAStartup (MAKEWORD (2, 2), &wsaData) != 0)
	{
		throw; //waip 
	}

	InitializeCriticalSection(&CritSectInPack);
	InitializeCriticalSection(&CritSectPrepareList);
	InitializeCriticalSection(&CritSectWaitAck);
};

TNetworkCore::~TNetworkCore(void)
{
	Terminate();
	DeleteCriticalSection(&CritSectInPack);
	DeleteCriticalSection(&CritSectPrepareList);
	DeleteCriticalSection(&CritSectWaitAck);
};


bool TNetworkCore::Setup(const char* Ip,unsigned short Port)
{
	TheSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if( TheSocket != INVALID_SOCKET )
	{
		// Initialize the socket with our adress and a random port
		BindSockAddr.sin_family = AF_INET;
		BindSockAddr.sin_port = htons( (GetTickCount()%30720)+2048 );

		BindSockAddr.sin_addr.S_un.S_addr = htonl( INADDR_ANY );
		if( BindSockAddr.sin_addr.S_un.S_addr == INADDR_NONE )
		{              
			closesocket( TheSocket );
			TheSocket = 0;
			return false;
		}

		SendSockAddr.sin_family = AF_INET;
		SendSockAddr.sin_port = htons(Port);
		SendSockAddr.sin_addr.S_un.S_addr = inet_addr( Ip );


		//bind setup the SOURCE PORT and SOURCE IP,  dest ip and dest port are indicated with sendto
		if( bind( TheSocket, (sockaddr *)&BindSockAddr, sizeof( sockaddr_in ) ) == 0 )
		{
			int SockLen=sizeof(sockaddr_in);
			// Fetch the adress set by the system.
			getsockname( TheSocket, (sockaddr *)&BindSockAddr, &SockLen );
			return true;
		}
	}
	closesocket( TheSocket );
	TheSocket = 0;
	return false;
};

long ThreadWrapperReceive(TNetworkCore *Ntw)
{
	Ntw->ThreadReceive();
	return 0;
};

long ThreadWrapperSend(TNetworkCore *Ntw)
{
	Ntw->ThreadSend();
	return 0;
};

void TNetworkCore::Connect(void)
{
	//launch processing threads

	unsigned long ThreadId;
	HdlThreadReceive=CreateThread(0,0,(LPTHREAD_START_ROUTINE)ThreadWrapperReceive,(void*)this,0,&ThreadId);
	HdlThreadSend=CreateThread(0,0,(LPTHREAD_START_ROUTINE)ThreadWrapperSend,(void*)this,0,&ThreadId);
}

void TNetworkCore::StoreInputPacket(TPacket* Packet)
{
	EnterCriticalSection(&CritSectInPack);
	
	InputPackList.push_back(Packet); //Ready for consumption

	LeaveCriticalSection(&CritSectInPack);
};

TPacket* TNetworkCore::GetNextPacket(void)
{
	EnterCriticalSection(&CritSectInPack);
	
	TPacket* Packet=0;
	if (!InputPackList.empty())
	{
		Packet=InputPackList.back(); //Ready for consumption
		InputPackList.pop_back();
	}

	LeaveCriticalSection(&CritSectInPack);
	return Packet;
};

void TNetworkCore::ThreadReceive(void)
{
	while (!ThreadReceiveTerminated)
	{
		//= 0 just to remove the warning...
		unsigned char Buffer[512]={0};

		int BufferLength=recvfrom(TheSocket, (char*)Buffer, 512, 0,(sockaddr*)&BindSockAddr,&SockLen);


		//keep track of the time of reception of that packet to be able to know if connection is alive
		LastReceiveStamp=GetTickCount();

		if (BufferLength>=DataGramHeaderSize)//everything below is junk for us
		{
			//Make a buffer and use it to feed the beast
			TDatagramBuffer* DatagramBuf=new TDatagramBuffer(Buffer,BufferLength);
			DatagramBuf->TimeStamp=LastReceiveStamp; //setup the timestamp for ping computation
			UseInputBuffer(DatagramBuf);

			//received something, reset timeout
			WaitTimeoutTime=0;
		} else
		if (BufferLength<0)
		{
			//receive error
		}
	}
};
	

void TNetworkCore::Acknowledge(const unsigned short UniqueID,const unsigned short FragmentNum,const unsigned long TimeStamp)
{
	EnterCriticalSection(&CritSectWaitAck);
	TDatagramBuffer* DtgBuf=(TDatagramBuffer*)WaitingForAckPool.GetEntry(FragHash(UniqueID,FragmentNum));
	if (DtgBuf!=0)
	{
		//compute the ping with the two timestamp
		const unsigned long TripTime=TimeStamp-DtgBuf->TimeStamp;
		if (TripTime<MinPing)
			MinPing=TripTime;
		if (TripTime>MaxPing)
			MaxPing=TripTime;
		//TODO we should give a greater weigth to AvgPing to have a smooth average update
		AvgPing=(AvgPing+TripTime)>>1;

		//remove it
		WaitingForAckPool.RemoveItem(FragHash(UniqueID,FragmentNum));
	}
	LeaveCriticalSection(&CritSectWaitAck);
};

void TNetworkCore::UseInputBuffer(TDatagramBuffer* DatagramBuf)
{
	if (!DatagramBuf->VerifyCrc16())
	{
		//blah !
		OutputDebugStringA("Failed crc test in TNetworkCore::UseInputBuffer\\n");
		delete DatagramBuf;
		return;
	}
	//check the header
	TDatagramHeader* DataHeader=(TDatagramHeader*)(DatagramBuf->Buffer);
	
	if (DataHeader->Ack==1)
	{
		//This is an Ack, search for the correlated Datagram
		Acknowledge(DataHeader->UniqueID,DataHeader->FragmentNumber,DatagramBuf->TimeStamp);
		//release memory
		delete DatagramBuf;
		return;
	}

	//this is not an ack, therefore it needs to be acknowledge
	TDataGram* AckDatagram=new TDataGram();
	AckDatagram->BuildAck(DataHeader->UniqueID,DataHeader->FragmentNumber);

	EnterCriticalSection(&CritSectPrepareList);
	PreparationList.push_back(AckDatagram);
	LeaveCriticalSection(&CritSectPrepareList);


	TDataGram* Datagram=0;
	if (DataHeader->Fragment==1)
	{
		//search for corresponding Datagram
		Datagram=(TDataGram*)FragmentList.GetEntry(FragHash(DataHeader->UniqueID,0));
		if (Datagram!=0)
		{
			//add the fragment 
			Datagram->AddBuffer(DatagramBuf);
		}else
		{
			//we need to build a new datagram to hold it
			Datagram=new TDataGram();
			Datagram->AddBuffer(DatagramBuf);
			FragmentList.AddHashEntry(FragHash(DataHeader->UniqueID,0),Datagram);
		}

		if (!Datagram->HasAllFragment())
			return;

		//TODO  remove datagram from list and process it if we have everything
		FragmentList.RemoveItem(FragHash(DataHeader->UniqueID,0));

	} else
	{
		//the buffer is not part of a fragmented pack
		Datagram=new TDataGram();
		Datagram->AddBuffer(DatagramBuf);
	}

	Datagram->MakePackets();
	TPacket* Packet;
	while ((Packet=Datagram->GetPacket())!=0)
		StoreInputPacket(Packet);

	
	//we don't need it anymore (or it's bad)
	delete Datagram;
};

void TNetworkCore::ThreadSend(void)
{
	unsigned int CheckAckCounter=0;

	//record the time passed without network activity
	WaitTimeoutTime=0;

	while (!ThreadSendTerminated)
	{
		//we sleep a little bit to wait for pak to be "Agglomerated"
		Sleep(PakAggloWaitTime); 

		//cycle through preparation list and send
		EnterCriticalSection(&CritSectPrepareList);

		if (PreparationList.size()==0)
		{
			WaitTimeoutTime+=PakAggloWaitTime;
			if (WaitTimeoutTime>(ClientTimeOut>>1))
			{
				if (WaitTimeoutTime<ClientTimeOut)
				{
					//send a keepalive
				//	OutputDebugStringA("Sent Keep Alive.\r\n");
					SendPacket(new TPacket(RQ_KeepAlive));
				}else
				{
					//We Timed out !!!
					//TODO Do somethign here...
				}
			}
		}

		//for each datagram
		for (unsigned int i=0; i< PreparationList.size();i++)
		{
			//gather one datagram and prepare it
			TDataGram* Dtg=PreparationList[i];
			Dtg->PrepareDatagram();

			TDatagramBuffer* DtgBuf;
			//cycle trough all buffers
			while (	(DtgBuf=Dtg->GetNextBuffer())!=0 )
			{
				DtgBuf->TimeStamp=GetTickCount();//Setup the timestamp
				//send it
				sendto(TheSocket,(char*)DtgBuf->Buffer,DtgBuf->BufferLength,0,(sockaddr*)&SendSockAddr,SockLen);

				//if it's not an ack store it in the waiting for ack list
				if (DtgBuf->BufferLength>DataGramHeaderSize)
				{
					TDatagramHeader* Hdr=(TDatagramHeader*)DtgBuf->Buffer;
					EnterCriticalSection(&CritSectWaitAck);
					WaitingForAckPool.AddHashEntry(FragHash(Hdr->UniqueID,Hdr->FragmentNumber),DtgBuf);
					LeaveCriticalSection(&CritSectWaitAck);
				} else
				{
					//just delete it otherwise
					delete DtgBuf;
				}
			}

			//destroy the TDatagram
			delete Dtg; 
		}
		//we consummed everything, clear the list
		PreparationList.clear();

		LeaveCriticalSection(&CritSectPrepareList);

		//check for datagrambuf without received ack
		//we don't check every iteration, useless
		unsigned long ActualTime=GetTickCount(); 
		if ((CheckAckCounter++%4)==0)
		{
			EnterCriticalSection(&CritSectWaitAck);
			WaitingForAckPool.ResetCycling();
			while(WaitingForAckPool.GetNextEntry()!=0)
			{
				TDatagramBuffer* DtgBuf=(TDatagramBuffer*)WaitingForAckPool.GetActualEntry();
				if ((ActualTime-DtgBuf->TimeStamp)>200)
				{
					DtgBuf->Retries++;
					DtgBuf->TimeStamp=ActualTime;
					//resend it
					sendto(TheSocket,(char*)DtgBuf->Buffer,DtgBuf->BufferLength,0,(sockaddr*)&SendSockAddr,SockLen);

					if (DtgBuf->Retries>=MaxSendRetry)
					{
						TDatagramHeader* Hdr=(TDatagramHeader*)DtgBuf->Buffer;
						WaitingForAckPool.RemoveItem(FragHash(Hdr->UniqueID,Hdr->FragmentNumber));
						delete DtgBuf;
					} 
				}

			}
			LeaveCriticalSection(&CritSectWaitAck);
		}
	}
};

void TNetworkCore::SendPacket(TPacket* Packet)
{
	//scan the list of datagram and find and appropriate one to store the packet
	//if not possible, make a new datagram
	EnterCriticalSection(&CritSectPrepareList);

	//cycle through datagrams
	bool Added=false;
	for(unsigned int i=0;i<PreparationList.size();i++)
	{
		//TODO is not able to mix ack with anythign for now
		if (PreparationList[i]->GetHeader().Ack==0)
		{
			//try to add
			if (PreparationList[i]->AddPacket(Packet))
			{
				Added=true;
				break;
			}
		}
	}

	if (!Added)
	{
		//no suitable datagram was found to add the packet
		TDataGram* Dtg=new TDataGram();
		Dtg->AddPacket(Packet);
		PreparationList.push_back(Dtg);
	}

	LeaveCriticalSection(&CritSectPrepareList);
}; 

bool TNetworkCore::IsConnectionAlive(void)
{
	//TODO Write the function..
	return true;
};
	
bool TNetworkCore::SlowTraffic(void)
{
	//TODO Write the function..
	return false;
};

void TNetworkCore::Terminate(void)
{
	ThreadSendTerminated=true;
	ThreadReceiveTerminated=true;
	shutdown(TheSocket,2);

	WaitForSingleObject(HdlThreadSend,500);
	WaitForSingleObject(HdlThreadReceive,500);
};