#ifndef DATAGRAM_H
#define DATAGRAM_H

#include "Headers.h"
#include "../ProtoHdr.h"


const int IP_HeaderSize=20;
const int UDP_HeaderSize=8;
const int MaxDatagramSize=512-UDP_HeaderSize-IP_HeaderSize; //"secure" & conservative datagram size

//forward declaration 
class TPacket;


struct TDatagramBuffer
{
	unsigned char* Buffer;
	int BufferLength;
	unsigned long TimeStamp;
	int Retries;

	unsigned int Position; //Ptr position

	TDatagramBuffer(void):Buffer(0),BufferLength(0),TimeStamp(0),Retries(0),Position(0){};
	TDatagramBuffer(const void* FromBuffer,const unsigned int FromBufLength);
	~TDatagramBuffer(void){delete [] Buffer;};

	void ResetPosition(void){Position=0;};
	//that function does not reallocate anything !!!! (just copy data)
	void AddData(const void* FromBuffer,const unsigned int FromBufLength);

	void WriteCrc16(void);
	//this destroys the crc value, and can only be executed once
	bool VerifyCrc16(void);
};


//a class to manage the creation of datagram from paks and the creation of Paks from datagram
class TDataGram
{
private:
	bool IsPrepared;

	int TotalSize;
	std::vector<TPacket*> PackList;
	std::vector<TDatagramBuffer*> BufferList;

	TDatagramHeader DataHeader;
public:
	TDataGram(void);
	~TDataGram(void);

	//function to build a new datagram from packets
	bool AddPacket(TPacket* Packet);				//add a packet to that datagram //return false if it's not possible to add
	TDatagramHeader& GetHeader(void){return DataHeader;};
	void BuildAck(const unsigned short UniqueID,const unsigned short FragmentNum);   //make an ack
	void PrepareDatagram(void);						//compile the packet list and make the headers
	TDatagramBuffer* GetNextBuffer(void);  //once the thing is prepared , retrieve one buffer

	//function to build packets from a datagram
	void AddBuffer(TDatagramBuffer* DatagramBuf);	//Add a buffer to reconstruct a packet
	bool HasAllFragment(void);						//check if we have all needed fragments
	void MakePackets(void);							//process the buffer and turn those into packets , return false if data are invalid
	unsigned int PacketCount(void);					//return the number of made packet
	//void ProcessHeader(void);						//fill the header struct with data from buffer
	unsigned short UniqueID(void);					//return the pak unique ID
	bool NeedMoreData(void);						//return true if it needs more data to be decoded
	TPacket* GetPacket(void);
};

#endif