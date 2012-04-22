#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include "Headers.h"

typedef std::vector<unsigned long> Utf32String;
Utf32String ConvertAsciiToUtf32(const char* Ascii);
Utf32String ConvertUtf16ToUtf32(std::wstring& Str);

std::string Convert(std::wstring& Str);
std::wstring Convert(std::string& Str);

std::vector<std::wstring> Tokenize(std::wstring &Str,wchar_t Delim,const bool trimEmpty=true);

#endif