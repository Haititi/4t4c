#ifndef PROTOHDR_H
#define PROTOHDR_H


/* pak struct
	if the pack is an ack, the total size to read is the TDatagramHeader  (4 bytes)

	if the pak is fragmented we obviously have only one long pak
		in that case all the fragments have the SAME uniqueID,
		and one ack is needed by fragment (finalID= uniqueID+FragmentNumber<<16);
*/
#pragma pack(push)
#pragma pack(1)
struct TDatagramHeader
{
	unsigned short UniqueID;//unique identifier of that pack
	unsigned short 	
		Ack:1,				//is an ack  (if true, the pack will only contain the header)
		Fragment:1,			//is a fragment  or the head of a fragmented pack
		FragmentNumber:4,	//ordinal of that fragment  // ==0 if this is the head obviously
		FragmentCount:4,	//as it says...
		Compression:2;		//type of compression
	unsigned short Crc16;  //the crc16 donc include itself of course... BUT it includes the header itself
};
const int DataGramHeaderSize=6;

struct TMultiPakHeader
{
	unsigned char PakCount; //number of pak in that datagram
};

const int MultiPakHeaderSize=1;

struct TPakHeader
{
	unsigned short PakLength;
	unsigned short PakType;
};

const int PakHeaderSize=4;

#pragma pack(pop)


//Misc network config
const int MaxSendRetry=4;

#endif