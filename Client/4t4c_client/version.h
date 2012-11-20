#ifndef VERSION_H
#define VERSION_H

#include "Headers.h"

namespace Version
{
    std::wstring&	GetVersionText(void);
    unsigned long	GetVersion(void);
    int				GetSubVersion(void);
};

#endif