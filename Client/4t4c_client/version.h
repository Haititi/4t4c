#ifndef VERSION_H
#define VERSION_H

#include "Headers.h"

namespace Version
{
    const char   *GetVersionText();
    unsigned long GetVersion();
    const char   *GetBuildTime();
    int         GetSubVersion();
};

#endif