#include "Datagram.h"
#include <windows.h>
#include "Packet.h"
#include <algorithm>

const unsigned short CrcTable[256]=
{
	0x0000,0xC0C1,0xC181,0x0140,0xC301,0x03C0,0x0280,0xC241,
	0xC601,0x06C0,0x0780,0xC741,0x0500,0xC5C1,0xC481,0x0440,
	0xCC01,0x0CC0,0x0D80,0xCD41,0x0F00,0xCFC1,0xCE81,0x0E40,
	0x0A00,0xCAC1,0xCB81,0x0B40,0xC901,0x09C0,0x0880,0xC841,
	0xD801,0x18C0,0x1980,0xD941,0x1B00,0xDBC1,0xDA81,0x1A40,
	0x1E00,0xDEC1,0xDF81,0x1F40,0xDD01,0x1DC0,0x1C80,0xDC41,
	0x1400,0xD4C1,0xD581,0x1540,0xD701,0x17C0,0x1680,0xD641,
	0xD201,0x12C0,0x1380,0xD341,0x1100,0xD1C1,0xD081,0x1040,
	0xF001,0x30C0,0x3180,0xF141,0x3300,0xF3C1,0xF281,0x3240,
	0x3600,0xF6C1,0xF781,0x3740,0xF501,0x35C0,0x3480,0xF441,
	0x3C00,0xFCC1,0xFD81,0x3D40,0xFF01,0x3FC0,0x3E80,0xFE41,
	0xFA01,0x3AC0,0x3B80,0xFB41,0x3900,0xF9C1,0xF881,0x3840,
	0x2800,0xE8C1,0xE981,0x2940,0xEB01,0x2BC0,0x2A80,0xEA41,
	0xEE01,0x2EC0,0x2F80,0xEF41,0x2D00,0xEDC1,0xEC81,0x2C40,
	0xE401,0x24C0,0x2580,0xE541,0x2700,0xE7C1,0xE681,0x2640,
	0x2200,0xE2C1,0xE381,0x2340,0xE101,0x21C0,0x2080,0xE041,
	0xA001,0x60C0,0x6180,0xA141,0x6300,0xA3C1,0xA281,0x6240,
	0x6600,0xA6C1,0xA781,0x6740,0xA501,0x65C0,0x6480,0xA441,
	0x6C00,0xACC1,0xAD81,0x6D40,0xAF01,0x6FC0,0x6E80,0xAE41,
	0xAA01,0x6AC0,0x6B80,0xAB41,0x6900,0xA9C1,0xA881,0x6840,
	0x7800,0xB8C1,0xB981,0x7940,0xBB01,0x7BC0,0x7A80,0xBA41,
	0xBE01,0x7EC0,0x7F80,0xBF41,0x7D00,0xBDC1,0xBC81,0x7C40,
	0xB401,0x74C0,0x7580,0xB541,0x7700,0xB7C1,0xB681,0x7640,
	0x7200,0xB2C1,0xB381,0x7340,0xB101,0x71C0,0x7080,0xB041,
	0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,0x5280,0x9241,
	0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,0x5440,
	0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
	0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,
	0x8801,0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,
	0x4E00,0x8EC1,0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,
	0x4400,0x84C1,0x8581,0x4540,0x8701,0x47C0,0x4680,0x8641,
	0x8201,0x42C0,0x4380,0x8341,0x4100,0x81C1,0x8081,0x4040
};


inline unsigned short ComputeCrc16(unsigned char const* Buffer,const unsigned int BufferLength)
{
	//PROCEDURE TCRCCalculator.CRC16 (p: pByte; nbyte: WORD; VAR CRCvalue: WORD);

	unsigned short CrcValue=0;
	for(unsigned int i=0;i<BufferLength;i++)
	{
		CrcValue=(CrcValue>>8) ^ CrcTable[*Buffer ^ (CrcValue & 0xFF)];
		Buffer++;
	}
	return CrcValue;
};

#define VoidAddr(Obj) (unsigned int*)((unsigned int)Obj)

//thread safe Id Generator
volatile long IdGenerator=0;
unsigned short GetNewId(void)
{
	unsigned long Temp=InterlockedIncrement(&IdGenerator);
	return Temp & 0xFFFF;
};

TDatagramBuffer::TDatagramBuffer(const void* FromBuffer,const unsigned int FromBufLength):
	BufferLength(FromBufLength),TimeStamp(0),Retries(0),Position(0)
{
	Buffer=new unsigned char[BufferLength];
	memcpy(Buffer,FromBuffer,BufferLength);
}

void TDatagramBuffer::AddData(const void* FromBuffer,const unsigned int FromBufLength)
{
	if (FromBufLength!=0)
	{
		memcpy(VoidAddr(Buffer+Position),FromBuffer,FromBufLength);
		Position+=FromBufLength;
	}
};


void TDatagramBuffer::WriteCrc16(void)
{
	//make sure the value is zero before computing
	((unsigned short*)Buffer)[2]=0;
	//compute and write
	((unsigned short*)Buffer)[2]=ComputeCrc16(Buffer,BufferLength);
};
	
bool TDatagramBuffer::VerifyCrc16(void)
{
	//save the value
	unsigned short VerifCrc16=((unsigned short*)Buffer)[2];
	//set the crc inside the buffer to zero 
	((unsigned short*)Buffer)[2]=0;
	return (VerifCrc16==ComputeCrc16(Buffer,BufferLength));
};


TDataGram::TDataGram(void):TotalSize(0),IsPrepared(false)
{
	DataHeader.Ack=0;
};

TDataGram::~TDataGram(void)
{
	//delete the associated memory

	//if we have some packet left inside (should not)
	while (!PackList.empty())
	{
		TPacket* Packet=PackList.back();
		delete Packet;
		PackList.pop_back();
	}

	//if we have some buffer left
	while (!BufferList.empty())
	{
		TDatagramBuffer* Buffer=BufferList.back();
		delete Buffer;
		BufferList.pop_back();
	}
};


bool TDataGram::AddPacket(TPacket* Packet)
{
	//if adding that packet would force to fragment and we already have some packet inside -> reject it
	int OldSize=TotalSize;
	if (!PackList.empty())
	{
		//we already have some data here, and we don't want to go past the limit
		if ((TotalSize+PakHeaderSize+Packet->GetBufferSize())>MaxDatagramSize)
			return false;
	} else
	{
		//the datagram is empty we need to add the multipakheader size
		TotalSize+=DataGramHeaderSize+MultiPakHeaderSize;
	}
	
	//otherwise just add that pack to the list  and compute the new size
	PackList.push_back(Packet);
	TotalSize+=PakHeaderSize+Packet->GetBufferSize();

	//check if we crossed a boundary and add the size of another DatagramHeader
	int SizeDiff=(TotalSize/MaxDatagramSize)-(OldSize/MaxDatagramSize);
	TotalSize+=DataGramHeaderSize*SizeDiff;
	return true;
};

void TDataGram::BuildAck(const unsigned short UniqueID,const unsigned short FragmentNum)
{
	DataHeader.UniqueID=UniqueID;
	DataHeader.Ack=1;
	DataHeader.Fragment=0;
	DataHeader.FragmentNumber=FragmentNum;
	DataHeader.FragmentCount=0;
	DataHeader.Compression=0;
};

void TDataGram::PrepareDatagram(void)
{
	if (!IsPrepared)
	{
		if (DataHeader.Ack==1)
		{
			//this is an ack
			TDatagramBuffer* DatagramBuf=new TDatagramBuffer(&DataHeader,DataGramHeaderSize);
			//don't need to set ack and timestamp since IT IS an ack
			DatagramBuf->WriteCrc16();
			//store it
			BufferList.push_back(DatagramBuf);
			
		} else
		{
			DataHeader.UniqueID=GetNewId();
			DataHeader.Crc16=0;
			DataHeader.Compression=0;
			//2 cases,  frag or not

			if (TotalSize<=MaxDatagramSize)
			{
				DataHeader.Fragment=0;
				DataHeader.FragmentNumber=0;
				DataHeader.FragmentCount=0;

				unsigned int Position=0;
				//make new buffer
				TDatagramBuffer* DatagramBuf=new TDatagramBuffer();
				DatagramBuf->BufferLength=TotalSize;
				DatagramBuf->Buffer=new unsigned char[TotalSize];

				//we will copy the real crc16 later
				DatagramBuf->AddData(&DataHeader,DataGramHeaderSize);
				
				//copy multipakHeader
				TMultiPakHeader MPakHdr;
				MPakHdr.PakCount=PackList.size();

				DatagramBuf->AddData(&MPakHdr,MultiPakHeaderSize);
				
				//for each packet
				for (unsigned int i=0;i<PackList.size();i++)
				{
					//make header
					TPakHeader PakHdr;
					PakHdr.PakType=PackList[i]->GetPakId();
					PakHdr.PakLength=PackList[i]->GetBufferSize();
					
					//copy header
					DatagramBuf->AddData(&PakHdr,PakHeaderSize);

					//copy Buffer
					DatagramBuf->AddData(PackList[i]->GetBuffer(),PackList[i]->GetBufferSize());
				}

				//write the crc
				DatagramBuf->WriteCrc16();

				//store the Datagrambuff
				BufferList.push_back(DatagramBuf);

			}else
			{
				//need fragmentation
				DataHeader.Fragment=1;
				DataHeader.FragmentNumber=0;
				DataHeader.FragmentCount=((TotalSize+(MaxDatagramSize>>1))/MaxDatagramSize)+1; 

				unsigned int SizeLeft=TotalSize;
				//we should have only one packet
				TPacket* Packet=PackList[0];
				unsigned int PackBuffPos=0;
				
				while(true)
				{
					TDatagramBuffer* DatagramBuf=new TDatagramBuffer();
					if (SizeLeft>=MaxDatagramSize)
						DatagramBuf->BufferLength=MaxDatagramSize;
					else
						DatagramBuf->BufferLength=SizeLeft;
					SizeLeft-=DatagramBuf->BufferLength;
					DatagramBuf->Buffer=new unsigned char[DatagramBuf->BufferLength];

					DatagramBuf->AddData(&DataHeader,DataGramHeaderSize);

					//if this is the first fragment
					if (DataHeader.FragmentNumber==0)
					{
						//add mpak header + pak header
						TMultiPakHeader MPakHdr;
						MPakHdr.PakCount=1;
						DatagramBuf->AddData(&MPakHdr,MultiPakHeaderSize);
						
						TPakHeader PakHdr;
						PakHdr.PakLength=Packet->GetBufferSize();
						PakHdr.PakType=Packet->GetPakId();
						DatagramBuf->AddData(&PakHdr,PakHeaderSize);
					}
					
					//fill the buffer with packet data
					unsigned int CopySize=DatagramBuf->BufferLength-DatagramBuf->Position;
					DatagramBuf->AddData(VoidAddr(Packet->GetBuffer()+PackBuffPos),CopySize);
					PackBuffPos+=CopySize;

					DatagramBuf->WriteCrc16();
					//Store it
					BufferList.push_back(DatagramBuf);

						
					if (SizeLeft==0)
						break;

					DataHeader.FragmentNumber++;
				}
			}
		}
	}
	
	IsPrepared=true;
};

void TDataGram::AddBuffer(TDatagramBuffer* DatagramBuf)
{
	BufferList.push_back(DatagramBuf);
};

bool TDataGram::HasAllFragment(void)
{
	if (!BufferList.empty())
	{
		TDatagramHeader* DtgHdr=(TDatagramHeader*)BufferList.front()->Buffer;
		if ((DtgHdr->FragmentCount+1) == BufferList.size())
			return true;
	}
	return false;
};

TDatagramBuffer* TDataGram::GetNextBuffer(void)
{
	TDatagramBuffer* Result=0;

	if (!BufferList.empty())
	{
		Result=BufferList.back();
		BufferList.pop_back();
	}

	return Result;
};

/*
void TDataGram::ProcessHeader(void)
{
	if (BufferList.size()>0)
	{
		DataHeader=*((TDatagramHeader*)(BufferList[0]->Buffer)); //copy data
	}
};*/


bool BufferSort(TDatagramBuffer* A,TDatagramBuffer* B) 
{ 
	return ((TDatagramHeader*)A->Buffer)->FragmentNumber<((TDatagramHeader*)B->Buffer)->FragmentNumber;
}

//take all input buffer (if several are present) assemble, and compute checksum
void TDataGram::MakePackets(void)
{
	if (!BufferList.empty())
	{

		TDatagramBuffer* DtBuf;
		DtBuf=BufferList.front();
		unsigned int Position=0;

		//if there is more than one buffer we need to sort & concatenate
		if (BufferList.size()>1)
		{
			//sort the buffer (in case of)
			std::sort(BufferList.begin(),BufferList.end(),BufferSort);
			//reassign it since we sorted
			DtBuf=BufferList.front();

			//reallocate coarsely
			DtBuf->Buffer=(unsigned char*)realloc(DtBuf->Buffer,BufferList.size()*512);

			Position=DtBuf->BufferLength;

			//we start at 1 since we copy in the first buff
			for (unsigned int i=1;i<BufferList.size();i++)
			{
				//we copy buffers withouth the datagramheader
				memcpy(VoidAddr(DtBuf->Buffer+Position),VoidAddr(BufferList[i]->Buffer+DataGramHeaderSize),BufferList[i]->BufferLength-DataGramHeaderSize);
				DtBuf->BufferLength+=BufferList[i]->BufferLength-DataGramHeaderSize;
			}
		}
	
		//we can have one or several packet in there
		Position=0;

		//we don't have enough data to fill the basic headers, just stop
		if (DataGramHeaderSize+MultiPakHeaderSize>DtBuf->BufferLength)	
			return;

		//copy main header data
		memcpy(&DataHeader,DtBuf->Buffer,DataGramHeaderSize);
		Position+=DataGramHeaderSize;

		//copy Multi pack header
		TMultiPakHeader MPakHdr;
		memcpy(&MPakHdr,VoidAddr(DtBuf->Buffer+Position),MultiPakHeaderSize);
		Position+=MultiPakHeaderSize;

	/*	if (MPakHdr.PakCount==4)
		{
			Sleep(0);
		}*/

		//"split" the buffer
		for (unsigned int i=0;i<MPakHdr.PakCount;i++)
		{
			if (Position+PakHeaderSize>DtBuf->BufferLength)	
				return;
			//read pak header
			TPakHeader PakHdr;
			memcpy(&PakHdr,VoidAddr(DtBuf->Buffer+Position),PakHeaderSize);
			Position+=PakHeaderSize;

			if (Position+PakHdr.PakLength>DtBuf->BufferLength)	
				return;
			//make one packet from buffer
			TPacket* NewPack=new TPacket(VoidAddr(DtBuf->Buffer+Position),PakHdr.PakLength);
			NewPack->SetPakId(PakHdr.PakType);
			Position+=PakHdr.PakLength;

			//store packet 
			PackList.push_back(NewPack);
		}
	} 
};

TPacket* TDataGram::GetPacket(void)
{
	TPacket* Result=0;
	
	if (!PackList.empty())
	{
		Result=PackList.back();
		PackList.pop_back();
	}
	return Result;
};