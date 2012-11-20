#include "version.h"

#include "rev.inc"
#define CLIENT_VERSION 100 

#define TOWIDE2(x) L ## x
#define TOWIDE(x) TOWIDE2(x)

#define __WDATE__ TOWIDE(__DATE__)
#define __WTIME__ TOWIDE(__TIME__)

#define BUILD_TIME __WDATE__ __WTIME__

namespace Version
{

std::wstring VersionStr(256,L'\0');


std::wstring& GetVersionText( void )
{
    if( VersionStr[ 0 ] == L'\0' )
	{
        swprintf_s( &VersionStr[0],256, L"4T4C %.2f at Rev %u -> %s", ((float)CLIENT_VERSION)/100.0f, REVISION_NUMBER, BUILD_TIME );
    }

    return VersionStr;
}

//return version number
unsigned long GetVersion( void )
{
    return CLIENT_VERSION;
}

//  Returns the revision
int GetSubVersion( void )
{
    return REVISION_NUMBER;
}

};

