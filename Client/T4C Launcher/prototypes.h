// prototypes.h

#ifndef PROTOTYPES_H
#define PROTOTYPES_H


// hyperlinks.c function prototypes
void ConvertStaticToHyperlink (HWND hwndParent, unsigned int wndcontrol_id);

// inifile.c function prototypes
void *INIFile_NewINIFile (void);
unsigned char INIFile_ReadEntryAsBool (void *ini_data, char *section, char *entry, unsigned char default_value);
long INIFile_ReadEntryAsLong (void *ini_data, char *section, char *entry, long default_value);
double INIFile_ReadEntryAsDouble (void *ini_data, char *section, char *entry, double default_value);
char *INIFile_ReadEntryAsString (void *ini_data, char *section, char *entry, char *default_value);
void INIFile_WriteEntryAsBool (void *ini_data, char *section, char *entry, unsigned char value);
void INIFile_WriteEntryAsLong (void *ini_data, char *section, char *entry, long value);
void INIFile_WriteEntryAsDouble (void *ini_data, char *section, char *entry, double value);
void INIFile_WriteEntryAsString (void *ini_data, char *section, char *entry, char *value);
void *INIFile_LoadINIFile (const char *filename);
void INIFile_SaveINIFile (const char *filename, void *ini_data);
void INIFile_FreeINIFile (void *ini_data);

// launcher.c function prototypes
void Loop_MOTD (void *thread_parms);
void Loop_LoginAndPatch (void);

// log.c function prototypes
void Log_Text (const char *logfile, const char *fmt, ...);
void Log_Hex (const char *logfile, const unsigned char *data, int data_size, const char *fmt, ...);
void Log_SQL (const char *logfile, int sqlcode, const char *fmt, ...);
void Log_Datagram (const char *logfile, datagram_t *datagram, const char *fmt, ...);
void Log_Pak (const char *logfile, pak_t *pak, const char *fmt, ...);

// network.c function prototypes
bool Network_Init (void);
void Network_Shutdown (void);
void Network_SendAll (void);
void Network_SendFromSocket (int socket_from);
bool Network_ReceiveAtSocket (datagram_t *datagram, int socket_at);
void Network_PrepareDatagram (datagram_t *datagram, bool is_reliable, int associated_id);
int SenderReceiver_New (int listenport);
void SenderReceiver_Destroy (int *senderreceiver_socket);

// pak.c function prototypes
void PakMachine_Init (void);
void PakMachine_Shutdown (void);
fragmented_pak_t *FragmentedPak_FindById (int id);
bool BuildPakFromUDPDatagrams (datagram_t *datagram, int crypto_version, pak_t *pak);
void BuildUDPDatagramsFromPak (pak_t *pak, int crypto_version, int socket_from, address_t *address_to);
void BuildAckReplyDatagramsFromPak (pak_t *pak, int crypto_version, int socket_from, address_t *address_to);
pak_t *Pak_New (int id, unsigned short flags);
void Pak_Destroy (pak_t *pak);
int8 Pak_ReadInt8 (pak_t *pak, unsigned int position);
int16 Pak_ReadInt16 (pak_t *pak, unsigned int position);
int32 Pak_ReadInt32 (pak_t *pak, unsigned int position);
int64 Pak_ReadInt64 (pak_t *pak, unsigned int position);
int Pak_ReadString8 (pak_t *pak, unsigned int position, char *out_string, int max_length);
int Pak_ReadString16 (pak_t *pak, unsigned int position, char *out_string, int max_length);
void Pak_WriteInt8 (pak_t *pak, bool update_size, unsigned int position, int8 in_data);
void Pak_WriteInt16 (pak_t *pak, bool update_size, unsigned int position, int16 in_data);
void Pak_WriteInt32 (pak_t *pak, bool update_size, unsigned int position, int32 in_data);
void Pak_WriteInt64 (pak_t *pak, bool update_size, unsigned int position, int64 in_data);
void Pak_WriteString8 (pak_t *pak, bool update_size, unsigned int position, char *fmt, ...);
void Pak_WriteString16 (pak_t *pak, bool update_size, unsigned int position, char *fmt, ...);
void Pak_WriteRawString8 (pak_t *pak, bool update_size, unsigned int position, int8 data_size, char *data);
void Pak_WriteRawString16 (pak_t *pak, bool update_size, unsigned int position, int16 data_size, char *data);
void Pak_AddInt8 (pak_t *pak, int8 in_data);
void Pak_AddInt16 (pak_t *pak, int16 in_data);
void Pak_AddInt32 (pak_t *pak, int32 in_data);
void Pak_AddInt64 (pak_t *pak, int64 in_data);
void Pak_AddString8 (pak_t *pak, char *fmt, ...);
void Pak_AddString16 (pak_t *pak, char *fmt, ...);
void Pak_AddRawString8 (pak_t *pak, int8 data_size, char *data);
void Pak_AddRawString16 (pak_t *pak, int16 data_size, char *data);

// proto160.c function prototypes
void Proto160_Init (void);
void Proto160_Shutdown (void);
bool Proto160_BuildPakFromUDPDatagrams (datagram_t *datagram, pak_t *pak);
void Proto160_BuildUDPDatagramsFromPak (pak_t *pak, int socket_from, address_t *address_to);
void Proto160_BuildAckReplyDatagramsFromPak (pak_t *pak, int socket_from, address_t *address_to);

// safelib.c function prototypes
void *SAFE_malloc (int count, size_t element_size, bool cleanup);
void *SAFE_realloc (void *allocation, int old_count, int new_count, size_t element_size, bool cleanup);
void SAFE_free (void **address_of_pointer_to_allocation);
char *SAFE_strncpy (char *destination, const char *source, size_t buffer_size);
int SAFE_snprintf (char *destination, size_t buffer_size, const char *fmt, ...);
int SAFE_vsnprintf (char *destination, size_t buffer_size, const char *fmt, va_list argptr);
char *SAFE_strncatf (char *destination, size_t buffer_size, const char *fmt, ...);
void SAFE_fclose (FILE **address_of_pointer_to_file);

// ui.c function prototypes
int WINAPI MainDialogProc (HWND hWnd, unsigned int message, unsigned int wParam, long lParam);
int WINAPI ConfigDialogProc (HWND hWnd, unsigned int message, unsigned int wParam, long lParam);

// util.c function prototypes
const char *Util_GetLastError (void);
const char *Util_GetLastNetworkError (void);
void Util_MakeAbsolutePathname (const char *pathname, char *absolute_pathname, int max_absolute_pathname_length);
unsigned long Util_LookupAddress (const char *address_as_string);
const char *Util_GetDirectoryPath (const char *pathname, char *path);
float ProcessTime (void);
bool WindowsShell_CopyDirectory (char *source_pathname, char *target_pathname);
bool WindowsShell_DeleteDirectory (char *pathname);
int MessageBox_printf (unsigned long mb_type, char *title, char *fmt, ...);


#endif // PROTOTYPES_H
