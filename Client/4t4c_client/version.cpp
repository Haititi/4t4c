#include "version.h"

#include "rev.inc"
#define CLIENT_VERSION 100 
#define BUILD_TIME __DATE__ " " __TIME__

namespace Version
{

char VersionStr[ 256 ] = { 0 };


const char *GetVersionText( void )
{
    if( VersionStr[ 0 ] == 0 )
	{
        sprintf_s( VersionStr,256, "4T4C %.2f at Rev %u -> %s", ((float)CLIENT_VERSION)/100.0f, REVISION_NUMBER, BUILD_TIME );
    }

    return VersionStr;
}

//return version number
unsigned long GetVersion( void )
{
    return CLIENT_VERSION;
}

//return build time as string
const char* GetBuildTime( void )
{
    return BUILD_TIME;
}

//  Returns the revision
int GetSubVersion( void )
{
    return REVISION_NUMBER;
}

};

