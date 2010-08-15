// client.h

#ifndef CLIENT_H
#define CLIENT_H


// multithreaded definition
#ifndef MT
#define MT
#endif


// win32 and standard C includes
//#define _CRT_SECURE_NO_WARNINGS // disable security warnings on strcpy
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <time.h>
#include <share.h>


// include stuff to make MSVC 6 ANSI C99 compliant
#include "c99.h"


// standard C does not recognize type 'bool'
#ifndef bool
#define bool unsigned char
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif


// don't use OS-dependent definitions
#ifdef MAX_PATH
#undef MAX_PATH
#endif


// our own includes
#include "paktypes.h"


// common defines
#define IS_SINGLEARG_PRESENT(arg) ((strnicmp (cmdline, arg, strlen (arg)) == 0) || (stristr (cmdline, " " arg) != NULL))


// languages
#define LANGUAGE_ENGLISH 0
#define LANGUAGE_FRENCH 1


// client screen resolutions
#define SCREENRESOLUTION_1024X768 0
#define SCREENRESOLUTION_800X600 1
#define SCREENRESOLUTION_640X480 2


// protocol-independent pak flags
#define PAKFLAG_NONE (unsigned char) 0
#define PAKFLAG_CRYPTO_VIRCOM (unsigned char) (1 << 0)
#define PAKFLAG_NEW_PAK (unsigned char) (1 << 2)
#define PAKFLAG_SERVER_PAK (unsigned char) (1 << 3)
#define PAKFLAG_CLIENT_PAK (unsigned char) (1 << 4)
#define PAKFLAG_ACKREQUESTED (unsigned char) (1 << 5)
#define PAKFLAG_ACKREPLY (unsigned char) (1 << 6)


// pak protocol versions
#define PAKCRYPTO_VERSION_160 160


// maximum string sizes
#define MAX_STRING8 0xff
#define MAX_STRING16 0xffff


// logging macros
#define LOG_MAIN "launcher.log"


// declaration for exported functions
#ifdef __cplusplus
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllexport)
#endif


// pak data type definitions
typedef unsigned char int8;
typedef unsigned short int16;
typedef unsigned long int32;
typedef unsigned __int64 int64;


// structure type definitions


// hashtables
typedef struct hashtable_entry_s
{
   void *element_address; // address of element in hashtable entry
} hashtable_entry_t;


typedef struct hashtable_bucket_s
{
   hashtable_entry_t *entries; // mallocated array of hasthable entries
   int size; // bucket element count
} hashtable_bucket_t;


// network addresses
typedef struct address_s
{
   char ip[16]; // IP address in full text, dotted style
   int port; // port number
} address_t;


// UDP datagrams
typedef struct datagram_s
{
   bool is_pending; // set to TRUE if this datagram should not be forgotten yet
   bool is_reliable; // set to TRUE if we are to wait for an ack reply to come back
   int associated_id; // associated ack ID (if we get an ack with same ID, datagram will be forgotten)
   int tries; // number of times this datagram was sent yet before the ack was gotten
   float send_date; // date at which this datagram should be sent again
   int our_socket; // socket from which to send this datagram, or at which it arrives
   address_t peer_address; // address where to direct this datagram, or where it comes from
   signed short size; // length of data (must be signed to handle negative error codes)
   char bytes[0x5aa]; // UDP data bytes (max datagram size cannot exceed ethernet MTU, which is 1450)
} datagram_t;


// fragmented paks
typedef struct fragmented_pak_s
{
   int id; // the fragmented pak's master id
   float arrival_date; // last received fragment's date of arrival
   char arrived_parts[32]; // array of flags telling which parts arrived and which not
   char parts_count; // number of parts this pak has
   short last_fragment_size; // the pak's last fragment raw data size
   char bytes[0xffff]; // full pak data, reassembled, starting right after the header
} fragmented_pak_t;


// T4C pak structure
typedef struct pak_s
{
   // WARNING: DATA ORDER IN THIS STRUCTURE AFFECTS PLUGIN INTERFACE CONSISTENCY

   int id; // this pak's ID (protocol independent)
   unsigned short flags; // pak flags used by proxy
   unsigned short data_size; // length of data
   int pages_count; // number of 1024-byte memory pages this pak data is spending
   char *data; // decrypted PAK data bytes, can't contain more than 65536 bytes (USHRT_MAX).
               // This space is mallocated, you shouldn't access it directly but only through
               // the ReadXXXXInPak()/WriteXXXXInPak() functions provided in the plugin API.
} pak_t;


// login pairs typedef
typedef struct login_s
{
   char username[64]; // player's username
   char password[64]; // player's password
} account_t;


// channel shortcut typedef
typedef struct cc_macro_s
{
   char shortcut[64]; // macro name, e.g. "rp"
   char channel[192]; // macro target channel, e.g. "Roleplay"
} channelshortcut_t;


// ini file data
typedef struct inifile_s
{
   char server_name[256];
   char server_url[256];
   char server_ip[32];
   char server_port[32];
   char accounts_lastlogin[32];
   account_t *accounts; // mallocated
   int account_count;
   int options_language;
   int options_screenresolution;
   int options_fullscreen;
   int options_keepaspectratio;
   int options_waitforvsync;
   int options_debuglevel;
   char shortcutkeys_inventory;
   char shortcutkeys_character;
   char shortcutkeys_chatwindow;
   char shortcutkeys_group;
   char shortcutkeys_spellbook;
   char shortcutkeys_macros;
   char shortcutkeys_exchange;
   char shortcutkeys_options;
   char shortcutkeys_screenshot;
   char shortcutkeys_bigchatwindow;
   char shortcutkeys_areamap;
   char shortcutkeys_attackmode;
   unsigned long *channelcolors; // mallocated
   int channelcolor_count;
   channelshortcut_t *channelshortcuts; // mallocated
   int channelshortcut_count;
} inifile_t;


// prototypes of global functions
#include "prototypes.h"


// global variables
#ifdef DEFINE_GLOBALS
#define GLOBAL 
#else
#define GLOBAL extern
#endif
GLOBAL char app_path[512];
GLOBAL HINSTANCE hAppInstance;
GLOBAL PROCESS_INFORMATION T4CExecutable_pi;
GLOBAL inifile_t config;
GLOBAL int our_socket;
GLOBAL char last_login[256];
GLOBAL char password[256];
GLOBAL char server_ip[32];
GLOBAL char server_port[32];
GLOBAL bool want_multiaccount;
GLOBAL bool want_nowebpatch;
GLOBAL int log_level;
GLOBAL float current_time;
GLOBAL address_t service_server;
GLOBAL datagram_t *datagrams; // datagrams array
GLOBAL int datagram_count;
GLOBAL char messagetext[MAX_STRING16];
GLOBAL bool messagetext_modified;


#endif // CLIENT_H