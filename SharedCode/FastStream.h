#ifndef FASTSTREAM_H
#define FASTSTREAM_H

#include <string>


class TFastStream
{
	private:
		unsigned int Granularity,Capacity;
		unsigned int Size;
		void *Memory,*Position;
		void RaiseCapacity(unsigned int Add);
	public:
		TFastStream(void);
		~TFastStream(void);
		void SetSize(unsigned int NewSize);
		inline unsigned int GetSize(void){return Size;};
		void SetGranularity(unsigned int Grain);
		void ResetPosition(void);
		void IncreasePos(unsigned int n);
		void Seek(unsigned int n);
		bool IsEnd(void){return (unsigned int)Position>=((unsigned int)Memory+Size);};
		bool LoadFromFile(std::wstring FileName);
		void SaveToFile(std::wstring FileName);

		void Write(void *Buffer,const long n);
	
		void WriteByte(unsigned char n);
		void WriteWord(unsigned short n);
		void WriteLong(unsigned long n);
		void WriteFloat(float n);
	
		void WriteByteString(const char *n);	
		void WriteWordString(const char *n);
		void WriteLongString(const char *n);

		void Read(void *Buffer,const long n);
		
		unsigned char ReadByte(void);
		unsigned short ReadWord(void);
		unsigned long ReadLong(void);
		float ReadFloat(void);

		char* ReadByteString(void);	
		char* ReadWordString(void);
		char* ReadLongString(void);

		char* ReadTextString(void);


};

#endif