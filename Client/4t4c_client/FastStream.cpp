#include "FastStream.h"


TFastStream::TFastStream(void)
{
	Size=0;
	Capacity=64;
	Granularity=64;
	Memory=malloc(Capacity);
	Position=Memory;

};

TFastStream::~TFastStream(void)
{
	if (Memory) free(Memory);
};


void TFastStream::SetSize(unsigned int NewSize)
{
	Memory=realloc(Memory,NewSize);
	Position=Memory;
	Capacity=NewSize;
	Size=NewSize;
};

void TFastStream::SetGranularity(unsigned int Grain)
{
	Granularity=Grain;
};

void TFastStream::RaiseCapacity(unsigned int Add)
{
	unsigned int addsize,intpos;
	intpos=(unsigned int) Position-(unsigned int) Memory;
    if (Add<Granularity) 
		addsize=Granularity; else 
		addsize=(Granularity-(Add%Granularity))+Add;
	Capacity+=addsize;
	Memory=realloc(Memory, Capacity);
	Position=(unsigned int*)((unsigned int) Memory+intpos); 
};

void TFastStream::ResetPosition()
{
	Position=Memory;
};

void TFastStream::IncreasePos(unsigned int n)
{
	Position=VoidAddr(Position+n);
};

void TFastStream::Seek(unsigned int n)
{
	Position=VoidAddr(Memory+n);
};

bool TFastStream::LoadFromFile(std::wstring FileName)
{
	HANDLE FileHdl;
	unsigned int FSize;
	unsigned long NbRead;
	FileHdl=CreateFileW(FileName.c_str(),GENERIC_ALL,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if (FileHdl==INVALID_HANDLE_VALUE)
		return false;
	FSize=GetFileSize(FileHdl,NULL);
	//todo : if getsize return $ffffffff  there is an error
	if (Capacity<FSize)
		SetSize(FSize);
	ReadFile(FileHdl,Memory,FSize,&NbRead,NULL);
	ResetPosition();
	//todo : should check nbread against filesize
	CloseHandle(FileHdl);
	return true;
}

void TFastStream::SaveToFile(std::wstring FileName)
{
	HANDLE FileHdl;
	unsigned long NbWrite;
	FileHdl=CreateFileW(FileName.c_str(),GENERIC_ALL,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	WriteFile(FileHdl,Memory,Size,&NbWrite,NULL);
	//todo : should check nbwrite against streamsize
	CloseHandle(FileHdl);
}

void TFastStream::Read(void *Buffer,const long n)
{
	memcpy(Buffer,Position,n);
	Position=VoidAddr(Position+n);
};


void TFastStream::Write(void *Buffer,const long n)
{
	Size+=n;
	if (Size>Capacity) RaiseCapacity(n);
	memcpy(Position,Buffer,n);
	Position=VoidAddr(Position+n);
};

void TFastStream::WriteByteString(const char *n)
{
	unsigned int StrSize=strlen(n);
	Size+=StrSize+1;
	if (Size>Capacity) RaiseCapacity(StrSize+1);
    *((unsigned char*)Position)=StrSize;
	memcpy(VoidAddr(Position+1),n,StrSize);
	Position=VoidAddr(Position+1+StrSize);
};

void TFastStream::WriteWordString(const char *n)
{
	unsigned int StrSize=strlen(n);
	Size+=StrSize+2;
	if (Size>Capacity) RaiseCapacity(StrSize+2);
    *(unsigned short*)Position=StrSize;
	memcpy(VoidAddr(Position+2),n,StrSize);
	Position=VoidAddr(Position+StrSize+2);
};

void TFastStream::WriteLongString(const char *n)
{
	unsigned int StrSize=strlen(n);
	Size+=StrSize+4;
	if (Size>Capacity) RaiseCapacity(StrSize+4);
    *(unsigned long*)Position=StrSize;
	memcpy(VoidAddr(Position+4),n,StrSize);
	Position=VoidAddr(Position+StrSize+4);
};

char* TFastStream::ReadByteString(void)
{
	unsigned char Size=*(unsigned char*)Position;
	char *n=new char[Size+1];
	Position=VoidAddr(Position+1);
	for (unsigned int i=0;i<Size;i++)
		n[i]=*((char*)VoidAddr(Position+i));
	n[Size]=0;
    Position=VoidAddr(Position+Size);
	return n;
};

char* TFastStream::ReadWordString(void)
{
	unsigned short Size=*(unsigned short*)Position;
	char *n=new char[Size+1];
	Position=VoidAddr(Position+2);
	for (unsigned int i=0;i<Size;i++)
		n[i]=*((char*)VoidAddr(Position+i));
	n[Size]=0;
	Position=VoidAddr(Position+Size);
	return n;
};

char* TFastStream::ReadLongString(void)
{
	unsigned long Size=*(unsigned long*)Position;
	char *n=new char[Size+1];
	Position=VoidAddr(Position+4);
	for (unsigned int i=0;i<Size;i++)
		n[i]=*((char*)VoidAddr(Position+i));
	n[Size]=0;
    Position=VoidAddr(Position+Size);
	return n;
};

char* TFastStream::ReadTextString(void)
{
	unsigned long Size=0;

	while(*((char*)VoidAddr(Position+Size))!=0x0D)
	{
		Size++;
	}

	char* Result=new char[Size+1];
	memcpy(Result,Position,Size);
	Result[Size]=0;

    Position=VoidAddr(Position+Size+2);
	return Result;
};

unsigned char TFastStream::ReadByte(void)
{
	unsigned char n=*(unsigned char*)Position;
    Position=VoidAddr(Position+sizeof(unsigned char));
	return n;
};
		
unsigned short TFastStream::ReadWord(void)
{
	unsigned short n=*(unsigned short*)Position;
    Position=VoidAddr(Position+sizeof(unsigned short));
	return n;
};
		
unsigned long TFastStream::ReadLong(void)
{
	unsigned long n=*(unsigned long*)Position;
    Position=VoidAddr(Position+sizeof(unsigned long));
	return n;
};
		
float TFastStream::ReadFloat(void)
{
	float n=*(float*)Position;
    Position=VoidAddr(Position+sizeof(float));
	return n;
};

void TFastStream::WriteByte(unsigned char n)
{
	Size+=sizeof(unsigned char);
	if (Size>Capacity) RaiseCapacity(sizeof(unsigned char));
	(*(unsigned char*)Position)=n;
	Position=VoidAddr(Position+sizeof(unsigned char));
};

void TFastStream::WriteWord(unsigned short n)
{
	Size+=sizeof(unsigned short);
	if (Size>Capacity) RaiseCapacity(sizeof(unsigned short));
	(*(unsigned short*)Position)=n;
	Position=VoidAddr(Position+sizeof(unsigned short));
};
		
void TFastStream::WriteLong(unsigned long n)
{
	Size+=sizeof(unsigned long);
	if (Size>Capacity) RaiseCapacity(sizeof(unsigned long));
	(*(unsigned long*)Position)=n;
	Position=VoidAddr(Position+sizeof(unsigned long));
};
		
void TFastStream::WriteFloat(float n)
{
	Size+=sizeof(float);
	if (Size>Capacity) RaiseCapacity(sizeof(float));
	(*(float*)Position)=n;
	Position=VoidAddr(Position+sizeof(float));
};