#ifndef FASTSTREAM_H
#define FASTSTREAM_H

#include <string>


class FastStream
{
	private:
		unsigned int Granularity,Capacity;
		unsigned int Size;
		void *Memory,*Position;
		void RaiseCapacity(unsigned int Add);
	public:
		FastStream(void);
		~FastStream(void);
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
	
		void WriteBool(bool n);
		void WriteByte(unsigned char n);
		void WriteWord(unsigned short n);
		void WriteLong(unsigned long n);
		void WriteFloat(float n);
		void WriteDouble(double n);
	
		void WriteWordString(const char *n);
		void WriteLongString(const char *n);

		void Read(void *Buffer,const long n);
		
		bool ReadBool(void);
		unsigned char ReadByte(void);
		unsigned short ReadWord(void);
		unsigned long ReadLong(void);
		float ReadFloat(void);
		double ReadDouble(void);

		char* ReadWordString(void);
		char* ReadLongString(void);

		char* ReadTextString(void);


		std::wstring ReadWideString(void);
		void WriteWideString(std::wstring& Value);

};

#endif