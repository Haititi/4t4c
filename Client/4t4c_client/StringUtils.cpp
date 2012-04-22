#include "StringUtils.h"
#include <sstream>

std::string convert(const std::wstring &Str)
{
    int StrLength = Str.length() + 1;
    int len = WideCharToMultiByte(CP_ACP, 0, Str.c_str(), StrLength, 0, 0, 0, 0); 
    std::string Result(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, Str.c_str(), StrLength, &Result[0], len, 0, 0); 
	return Result;
}

std::wstring convert(const std::string &Str)
{
	const int StrLength = (int)Str.length() + 1;
	const int Len = MultiByteToWideChar(CP_ACP, 0, Str.c_str(), StrLength, 0, 0); 
	std::wstring Result(Len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, Str.c_str(), StrLength, &Result[0], Len);
    return Result;
}

std::vector<std::wstring> Tokenize(std::wstring &Str,wchar_t Delim, const bool TrimEmpty)
{
	std::vector<std::wstring> Result;

	std::string::size_type Pos, LastPos = 0;

	while (true)
	{
		Pos = Str.find_first_of(Delim, LastPos);
		if(Pos == std::string::npos)
		{
			Pos = Str.length();

			if(Pos != LastPos || !TrimEmpty)
				Result.push_back(std::wstring(Str.data()+LastPos,(std::wstring::size_type)Pos-LastPos ));

			break;
		}
		else
		{
			if(Pos != LastPos || !TrimEmpty)
				Result.push_back(std::wstring(Str.data()+LastPos,(std::wstring::size_type)Pos-LastPos ));
		}

		LastPos = Pos + 1;
	}
	return Result;
};


Utf32String ConvertAsciiToUtf32(const char* Ascii)
{
	const int TmpBufferSize=512;
	wchar_t Tmp[TmpBufferSize];
	MultiByteToWideChar(CP_ACP, 0, Ascii, -1, Tmp, TmpBufferSize);

	Utf32String Utf32str;
	Utf32str.reserve(128);

	wchar_t* Ptr=Tmp;
	while((*Ptr)!=0)
	{
		Utf32str.push_back(*Ptr);
		Ptr++;
	}
	return Utf32str;
};


Utf32String ConvertUtf16ToUtf32(std::wstring& Str)
{
	Utf32String Utf32str;
	Utf32str.reserve(Str.size());
	for (unsigned int i=0;i<Str.size();i++)
	{
		Utf32str.push_back(Str[i]);
	}
	return Utf32str;
};