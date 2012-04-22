#ifndef VERSION_H
#define VERSION_H

#include "Headers.h"

namespace Version
{
    std::wstring& GetVersionText();
    unsigned long GetVersion();
    const wchar_t   *GetBuildTime();
    int         GetSubVersion();
};

#endif