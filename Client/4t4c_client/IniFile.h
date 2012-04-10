#ifndef INIFILE_H
#define INIFILE_H



// ini file function prototypes
void *INIFile_NewINIFile (void);
int INIFile_GetNumberOfSections (void *ini_data);
char *INIFile_GetSectionName (void *ini_data, int section_index);
unsigned char INIFile_ReadEntryAsBool (void *ini_data, char *section, char *entry, unsigned char default_value);
long INIFile_ReadEntryAsLong (void *ini_data, char *section, char *entry, long default_value);
double INIFile_ReadEntryAsDouble (void *ini_data, char *section, char *entry, double default_value);
char *INIFile_ReadEntryAsString (void *ini_data, char *section, char *entry, char *default_value);
void INIFile_WriteEntryAsBool (void *ini_data, char *section, char *entry, unsigned char value);
void INIFile_WriteEntryAsLong (void *ini_data, char *section, char *entry, long value);
void INIFile_WriteEntryAsDouble (void *ini_data, char *section, char *entry, double value);
void INIFile_WriteEntryAsString (void *ini_data, char *section, char *entry, char *value);
void INIFile_DeleteEntry (void *ini_data, char *section, char *entry);
void INIFile_DeleteSection (void *ini_data, char *section);
void *INIFile_LoadINIFile (const char *filename);
void INIFile_SaveINIFile (const char *filename, void *ini_data);
void INIFile_FreeINIFile (void *ini_data);

#endif