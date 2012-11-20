#include "FastStream.h"
#include <Windows.h>

#define VoidAddr(Obj) (unsigned int*)((unsigned int)Obj)

FastStream::FastStream(void)
{
	Size=0;
	Capacity=64;
	Granularity=64;
	Memory=malloc(Capacity);
	Position=Memory;

};

FastStream::~FastStream(void)
{
	if (Memory) free(Memory);
};


void FastStream::SetSize(unsigned int NewSize)
{
	Memory=realloc(Memory,NewSize);
	Position=Memory;
	Capacity=NewSize;
	Size=NewSize;
};

void FastStream::SetGranularity(unsigned int Grain)
{
	Granularity=Grain;
};

void FastStream::RaiseCapacity(unsigned int Add)
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

void FastStream::ResetPosition()
{
	Position=Memory;
};

void FastStream::IncreasePos(unsigned int n)
{
	Position=VoidAddr(Position+n);
};

void FastStream::Seek(unsigned int n)
{
	Position=VoidAddr(Memory+n);
};

bool FastStream::LoadFromFile(std::wstring FileName)
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

void FastStream::SaveToFile(std::wstring FileName)
{
	HANDLE FileHdl;
	unsigned long NbWrite;
	FileHdl=CreateFileW(FileName.c_str(),GENERIC_ALL,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	WriteFile(FileHdl,Memory,Size,&NbWrite,NULL);
	//todo : should check nbwrite against streamsize
	CloseHandle(FileHdl);
}

void FastStream::Read(void *Buffer,const long n)
{
	memcpy(Buffer,Position,n);
	Position=VoidAddr(Position+n);
};


void FastStream::Write(void *Buffer,const long n)
{
	Size+=n;
	if (Size>Capacity) RaiseCapacity(n);
	memcpy(Position,Buffer,n);
	Position=VoidAddr(Position+n);
};

void FastStream::WriteWordString(const char *n)
{
	unsigned int StrSize=strlen(n);
	Size+=StrSize+2;
	if (Size>Capacity) RaiseCapacity(StrSize+2);
    *(unsigned short*)Position=StrSize;
	memcpy(VoidAddr(Position+2),n,StrSize);
	Position=VoidAddr(Position+StrSize+2);
};

void FastStream::WriteLongString(const char *n)
{
	unsigned int StrSize=strlen(n);
	Size+=StrSize+4;
	if (Size>Capacity) RaiseCapacity(StrSize+4);
    *(unsigned long*)Position=StrSize;
	memcpy(VoidAddr(Position+4),n,StrSize);
	Position=VoidAddr(Position+StrSize+4);
};

char* FastStream::ReadWordString(void)
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

char* FastStream::ReadLongString(void)
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

char* FastStream::ReadTextString(void)
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

bool FastStream::ReadBool(void)
{
	unsigned char n=*(unsigned char*)Position;
    Position=VoidAddr(Position+sizeof(unsigned char));
	return n!=0;
};

void FastStream::WriteBool(bool n)
{
	Size+=sizeof(unsigned char);
	if (Size>Capacity) RaiseCapacity(sizeof(unsigned char));
	if (n)
		(*(unsigned char*)Position)=1;
	else
		(*(unsigned char*)Position)=0;
	Position=VoidAddr(Position+sizeof(unsigned char));
};

unsigned char FastStream::ReadByte(void)
{
	unsigned char n=*(unsigned char*)Position;
    Position=VoidAddr(Position+sizeof(unsigned char));
	return n;
};
		
unsigned short FastStream::ReadWord(void)
{
	unsigned short n=*(unsigned short*)Position;
    Position=VoidAddr(Position+sizeof(unsigned short));
	return n;
};
		
unsigned long FastStream::ReadLong(void)
{
	unsigned long n=*(unsigned long*)Position;
    Position=VoidAddr(Position+sizeof(unsigned long));
	return n;
};
		
float FastStream::ReadFloat(void)
{
	float n=*(float*)Position;
    Position=VoidAddr(Position+sizeof(float));
	return n;
};

void FastStream::WriteByte(unsigned char n)
{
	Size+=sizeof(unsigned char);
	if (Size>Capacity) RaiseCapacity(sizeof(unsigned char));
	(*(unsigned char*)Position)=n;
	Position=VoidAddr(Position+sizeof(unsigned char));
};

void FastStream::WriteWord(unsigned short n)
{
	Size+=sizeof(unsigned short);
	if (Size>Capacity) RaiseCapacity(sizeof(unsigned short));
	(*(unsigned short*)Position)=n;
	Position=VoidAddr(Position+sizeof(unsigned short));
};
		
void FastStream::WriteLong(unsigned long n)
{
	Size+=sizeof(unsigned long);
	if (Size>Capacity) RaiseCapacity(sizeof(unsigned long));
	(*(unsigned long*)Position)=n;
	Position=VoidAddr(Position+sizeof(unsigned long));
};
		
void FastStream::WriteFloat(float n)
{
	Size+=sizeof(float);
	if (Size>Capacity) RaiseCapacity(sizeof(float));
	(*(float*)Position)=n;
	Position=VoidAddr(Position+sizeof(float));
};

std::wstring FastStream::ReadWideString(void)
{
	unsigned long Size=ReadLong();
	std::wstring Result(Size,L'\0');
	Read(&Result[0],Size*sizeof(wchar_t));
	return Result;
};

void FastStream::WriteWideString(std::wstring& Str)
{
	WriteLong(Str.size());
	Write(&Str[0],Str.size()*sizeof(wchar_t));
};

void FastStream::WriteDouble(double n)
{
};

double FastStream::ReadDouble(void)
{
	return 0;
};