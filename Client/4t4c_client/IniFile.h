#ifndef INIFILE_H
#define INIFILE_H

#include "Headers.h"

// dictionary structure definitions
struct dictionary_entry_t
{
   wchar_t key[128]; // key string
   unsigned long hash; // key hash value
   wchar_t value[256]; // value string
};

struct dictionary_t
{
   int size; // storage size
   dictionary_entry_t *entries; // array of entries (mallocated)
   int entry_count; // number of entries in dictionary
};

class IniFile
{
private:
	dictionary_t* Dictionary;

	dictionary_t* CreateDictionary(int size);
	void DestroyDictionary (void);
	dictionary_entry_t *GetKey (const wchar_t *key);
	wchar_t* ReadKey (const wchar_t *key, wchar_t *default_value);
	void WriteKey (const wchar_t *key, wchar_t *value);
	void DeleteKey (const wchar_t *key);
	unsigned long ComputeHashValueForKey (const wchar_t *key);
public:
	IniFile(void);
	IniFile(std::wstring& FileName); //equivalent of Loadinifile
	~IniFile(void);

	void SaveFile (std::wstring& FileName);

	int GetNumberOfSections (void);
	wchar_t* GetSectionName (int section_index);
	bool ReadEntryAsBool (wchar_t *section, wchar_t *entry, bool default_value);
	long ReadEntryAsLong (wchar_t *section, wchar_t *entry, long default_value);
	double ReadEntryAsDouble (wchar_t *section, wchar_t *entry, double default_value);
	wchar_t* ReadEntryAsString (wchar_t *section, wchar_t *entry, wchar_t *default_value);
	void WriteEntryAsBool (wchar_t *section, wchar_t *entry, bool value);
	void WriteEntryAsLong (wchar_t *section, wchar_t *entry, long value);
	void WriteEntryAsDouble (wchar_t *section, wchar_t *entry, double value);
	void WriteEntryAsString (wchar_t *section, wchar_t *entry, wchar_t *value);
	void DeleteEntry (wchar_t *section, wchar_t *entry);
	void DeleteSection (wchar_t *section);
};

#endif