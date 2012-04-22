// inifile.c

#include <windows.h>
#include <stdio.h>
#include <share.h> 
#include "inifile.h"
#include "safelib.h"

// internal definitions
const int DICTIONARY_PAGE_SIZE=128;
const wchar_t INI_SECTION_SEPARATOR=L']';
#define INI_INVALID_KEY (wchar_t *) 0x7fffffff

// internal variables
static wchar_t *INIFile_default_section = L"__default";

dictionary_t* IniFile::CreateDictionary (int size)
{
	// this function allocates a new dictionary object of given size and returns it. If you do
	// not know in advance (roughly) the number of entries in the dictionary, give size = 0.

	dictionary_t *dictionary;

	// if no size was specified, allocate space for one page
	if (size < DICTIONARY_PAGE_SIZE)
		size = DICTIONARY_PAGE_SIZE;

	// allocate one instance of the dictionary
	dictionary = (dictionary_t *) malloc (sizeof (dictionary_t));

	dictionary->entry_count = 0; // no entries in dictionary yet
	dictionary->size = size; // dictionary can currently hold size entries

	// allocate space for the dictionary entries and zero all the crap out
	dictionary->entries = (dictionary_entry_t *) malloc (size * sizeof (dictionary_entry_t));
	memset (dictionary->entries, 0, size * sizeof (dictionary_entry_t));

	return (dictionary); // finished, return a pointer to the dictionary object
}

void IniFile::DestroyDictionary (void)
{
	// frees a dictionary object and all memory associated to it.

	free (Dictionary->entries); // free the entries array
	free (Dictionary); // and finally, free the dictionary itself
}

dictionary_entry_t *IniFile::GetKey (const wchar_t *key)
{
	// this function locates a key in a dictionary and returns a pointer to it, or a NULL
	// pointer if no such key can be found in dictionary. The returned pointer points to data
	// internal to the dictionary object, do not try to free or modify it.

	unsigned long hash;
	register int index;

	hash = ComputeHashValueForKey (key); // get the hash value for key

	// cycle through all entries in the dictionary
	for (index = 0; index < Dictionary->entry_count; index++)
	{
		if (Dictionary->entries[index].key[0] == 0)
			continue; // skip empty slots

		// compare hash AND string, to avoid hash collisions
		if ((hash == Dictionary->entries[index].hash) && (wcscmp (key, Dictionary->entries[index].key) == 0))
			return (&Dictionary->entries[index]); // return a pointer to the key if we find it
	}

	return (NULL); // else return a NULL pointer
}


wchar_t *IniFile::ReadKey (const wchar_t *key, wchar_t *default_value)
{
	// this function locates a key in a dictionary and returns a pointer to its value, or the
	// passed 'def' pointer if no such key can be found in dictionary. The returned char pointer
	// points to data internal to the dictionary object, do not try to free or modify it.

	dictionary_entry_t *element;

	element = GetKey (key); // query the dictionary for the key
	if (element != NULL)
		return (element->value); // if we found a valid key, return the value associed to it

	return (default_value); // else return the default value
}


void IniFile::WriteKey (const wchar_t *key, wchar_t *value)
{
	// sets a value in a dictionary. If the given key is found in the dictionary, the associated
	// value is replaced by the provided one. If the key cannot be found in the dictionary, it
	// is added to it.

	unsigned long hash;
	register int index;

	hash = ComputeHashValueForKey (key); // compute hash for this key

	// for each entry in the dictionary...
	for (index = 0; index < Dictionary->entry_count; index++)
	{
		if (Dictionary->entries[index].key[0] == 0)
			continue; // skip empty slots

		// does that key have the same hash value AND the same name ?
		if ((hash == Dictionary->entries[index].hash) && (wcscmp (key, Dictionary->entries[index].key) == 0))
		{
			// we've found the right key: modify its value and return

			// have we provided a valid value ?
			if (value != NULL)
				wcscpy_s (Dictionary->entries[index].value, 256, value); // copy the value string
			else
				Dictionary->entries[index].value[0] = 0; // reset the value string

			return; // value has been modified: return
		}
	}

	// here either the dictionary was empty, or we couldn't find a similar value: add a new one

	// cycle through all entries in the dictionary...
	for (index = 0; index < Dictionary->entry_count; index++)
		if (Dictionary->entries[index].key[0] == 0)
			break; // and stop at the first empty one we find

	// no empty entry found, see if dictionary needs to grow or if there's still room left
	if (index == Dictionary->size)
	{
		// mallocate some more space for the dictionary and zero all that crap out
		Dictionary->entries = (dictionary_entry_t *) realloc (Dictionary->entries, (Dictionary->size + DICTIONARY_PAGE_SIZE) * sizeof (dictionary_entry_t)); // alloc 1 more page
		Dictionary->size += DICTIONARY_PAGE_SIZE; // increase dictionary size
	}

	// copy the new key

	wcscpy_s (Dictionary->entries[index].key, 128, key); // copy the key string...
	Dictionary->entries[index].hash = hash; // ...and store the hash value

	// have we provided a valid value ?
	if (value != NULL)
		wcscpy_s (Dictionary->entries[index].value, 256, value); // copy the value string
	else
		Dictionary->entries[index].value[0] = 0; // reset the value string

	Dictionary->entry_count++; // there is one more entry in the dictionary now
	return;
}


void IniFile::DeleteKey (const wchar_t *key)
{
	// this function deletes a key in a dictionary. Nothing is done if the key cannot be found.

	unsigned long hash;
	register int index;

	hash = ComputeHashValueForKey (key); // get the hash value for key

	// cycle through all entries in the dictionary...
	for (index = 0; index < Dictionary->entry_count; index++)
	{
		if (Dictionary->entries[index].key[0] == 0)
			continue; // skip empty slots

		// compare hash AND string, to avoid hash collisions
		if ((hash == Dictionary->entries[index].hash) && (wcscmp (key, Dictionary->entries[index].key) == 0))
			break; // break as soon as we've found the key we want
	}

	if (index == Dictionary->entry_count)
		return; // if key was not found, just return

	// clear the key, the hash and its value data
	memset (&Dictionary->entries[index], 0, sizeof (dictionary_entry_t));

	Dictionary->entry_count--; // there is one entry less in the dictionary now
	return;
}


unsigned long IniFile::ComputeHashValueForKey (const wchar_t *key)
{
	// Compute the hash key for a string. This hash function has been taken from an article in
	// Dr Dobbs Journal. This is normally a collision-free function, distributing keys evenly.
	// The key is stored anyway in the struct so that collision can be avoided by comparing the
	// key itself in last resort. THIS FUNCTION IS TO BE USED INTERNALLY ONLY!

	register unsigned long hash;
	int length;
	int index;

	hash = 0;
	length = (int) wcslen (key)*sizeof(wchar_t);

	unsigned char* Ptr=(unsigned char*)key;
	// for each character of the string...
	for (index = 0; index < length; index++)
	{
		hash += (unsigned long) Ptr[index]; // take it in account and compute the hash value
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3); // finalize hashing (what the hell does it do, I have no clue)
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return (hash); // and return the hash value
}

IniFile::IniFile(void)
{
	Dictionary=CreateDictionary(0);
};

IniFile::IniFile(std::wstring& FileName)
{
	// this is the parser for ini files. This function is called, providing the name of the file
	// to be read. It returns a dictionary object that should not be accessed directly, but
	// through accessor functions instead.

	FILE *fp;
	int fieldstart;
	int fieldstop;
	int length;
	int i;
	wchar_t line_buffer[1024];
	wchar_t section[256];
	wchar_t entry[256];
	wchar_t value[256];

	//we create the dictionnary 1st that way we guarantee  that it is non-null
	Dictionary = CreateDictionary (0);

	// try to open the INI file in ASCII read-only mode
	fp = _wfsopen (FileName.c_str(), L"r", _SH_DENYNO);
	if (fp == NULL)
		return; // cancel if file not found


	// set the default section for orphaned entries and add it to the dictionary
	wcscpy_s (section, 256, INIFile_default_section);
	WriteEntryAsString (section, NULL, NULL);

	// read line per line...
	while (fgetws (line_buffer, 1024, fp) != NULL)
	{
		length = (int) wcslen (line_buffer); // get line length

		while ((length > 0) && ((line_buffer[length - 1] == L'\n') || (line_buffer[length - 1] == L'\r')))
			length--; // discard trailing newlines

		fieldstart = 0; // let's now strip leading blanks
		while ((fieldstart < length) && iswspace ((unsigned char) line_buffer[fieldstart]))
			fieldstart++; // ignore any tabs or spaces, going forward from the start

		fieldstop = length - 1; // let's now strip trailing blanks
		while ((fieldstop >= 0) && iswspace ((unsigned char) line_buffer[fieldstop]))
			fieldstop--; // ignore any tabs or spaces, going backwards from the end

		for (i = fieldstart; i <= fieldstop; i++)
			line_buffer[i - fieldstart] = line_buffer[i]; // recopy line buffer without the spaces
		line_buffer[i - fieldstart] = 0; // and terminate the string

		if ((line_buffer[0] == ';') || (line_buffer[0] == '#') || (line_buffer[0] == 0))
			continue; // skip comment lines

		// is it a valid section name ?
		if (swscanf_s (line_buffer, L"[%[^]]", section, sizeof (section)) == 1)
		{
			length = (int) wcslen (section); // get the section string length

			fieldstart = 0; // let's now strip leading blanks
			while ((fieldstart < length) && iswspace ((unsigned char) section[fieldstart]))
				fieldstart++; // ignore any tabs or spaces, going forward from the start

			fieldstop = length - 1; // let's now strip trailing blanks
			while ((fieldstop >= 0) && iswspace ((unsigned char) section[fieldstop]))
				fieldstop--; // ignore any tabs or spaces, going backwards from the end

			for (i = fieldstart; i <= fieldstop; i++)
				section[i - fieldstart] = section[i]; // recopy section name w/out spaces
			section[i - fieldstart] = 0; // and terminate the string

			WriteEntryAsString (section, NULL, NULL); // add to dictionary
		}

		// else is it a valid entry/value pair that is enclosed between quotes?
		else if (swscanf_s (line_buffer, L"%[^=] = \"%[^\"]\"", entry, sizeof (entry), value, sizeof (value)) == 2)
		{
			length = (int) wcslen (entry); // get the entry string length

			fieldstart = 0; // let's now strip leading blanks
			while ((fieldstart < length) && iswspace ((unsigned char) entry[fieldstart]))
				fieldstart++; // ignore any tabs or spaces, going forward from the start

			fieldstop = length - 1; // let's now strip trailing blanks
			while ((fieldstop >= 0) && iswspace ((unsigned char) entry[fieldstop]))
				fieldstop--; // ignore any tabs or spaces, going backwards from the end

			for (i = fieldstart; i <= fieldstop; i++)
				entry[i - fieldstart] = towlower (entry[i]); // recopy entry name w/out spaces
			entry[i - fieldstart] = 0; // and terminate the string

			// when value is enclosed between quotes, DO NOT strip the blanks

			// sscanf cannot handle "" or '' as empty value, this is done here
			if ((wcscmp (value, L"\"\"") == 0) || (wcscmp (value, L"''") == 0))
				value[0] = 0; // empty string

			WriteEntryAsString (section, entry, value); // add to dictionary
		}

		// else is it a valid entry/value pair without quotes ?
		else if ((swscanf_s (line_buffer, L"%[^=] = '%[^\']'", entry, sizeof (entry), value, sizeof (value)) == 2)
			|| (swscanf_s (line_buffer, L"%[^=] = %[^;#]", entry, sizeof (entry), value, sizeof (value)) == 2))
		{
			length = (int) wcslen (entry); // get the entry string length

			fieldstart = 0; // let's now strip leading blanks
			while ((fieldstart < length) && iswspace ((unsigned short) entry[fieldstart]))
				fieldstart++; // ignore any tabs or spaces, going forward from the start

			fieldstop = length - 1; // let's now strip trailing blanks
			while ((fieldstop >= 0) && iswspace ((unsigned short) entry[fieldstop]))
				fieldstop--; // ignore any tabs or spaces, going backwards from the end

			for (i = fieldstart; i <= fieldstop; i++)
				entry[i - fieldstart] = towlower (entry[i]); // recopy entry name w/out spaces
			entry[i - fieldstart] = 0; // and terminate the string

			length = (int) wcslen (value); // get the value string length

			fieldstart = 0; // let's now strip leading blanks
			while ((fieldstart < length) && iswspace ((unsigned short) value[fieldstart]))
				fieldstart++; // ignore any tabs or spaces, going forward from the start

			fieldstop = length - 1; // let's now strip trailing blanks
			while ((fieldstop >= 0) && iswspace ((unsigned short) value[fieldstop]))
				fieldstop--; // ignore any tabs or spaces, going backwards from the end

			for (i = fieldstart; i <= fieldstop; i++)
				value[i - fieldstart] = value[i]; // recopy entry name w/out spaces
			value[i - fieldstart] = 0; // and terminate the string

			// sscanf cannot handle "" or '' as empty value, this is done here
			if ((wcscmp (value, L"\"\"") == 0) || (wcscmp (value, L"''") == 0))
				value[0] = 0; // empty string

			WriteEntryAsString (section, entry, value); // add to dictionary
		}
	}

	fclose (fp); // finished, close the file
};

IniFile::~IniFile(void)
{
	DestroyDictionary();
};


int IniFile::GetNumberOfSections (void)
{
	// this function returns the number of sections found in a dictionary. The test to recognize
	// sections is done on the string stored in the dictionary: a section name is given as
	// "section" whereas a key is stored as "section]key", thus the test looks for entries that
	// do NOT contain a ']'. This clearly fails in the case a section name contains a ']',
	// but this should simply be avoided.

	int section_count;
	int index;

	section_count = 0; // no sections found yet

	// for each entry in the dictionary...
	for (index = 0; index < Dictionary->entry_count; index++)
	{
		if (Dictionary->entries[index].key[0] == 0)
			continue; // skip empty slots

		if (wcschr (Dictionary->entries[index].key, INI_SECTION_SEPARATOR) == NULL)
			section_count++; // if this entry has NO section separator, then it's a section name
	}

	return (section_count); // return the section count we found
}


wchar_t *IniFile::GetSectionName (int section_index)
{
	// this function locates the n-th section in a dictionary and returns its name as a pointer
	// to a string allocated inside the dictionary. Do not free or modify the returned string!!
	// This function returns NULL in case of error.

	int sections_found;
	int index;

	if (section_index < 0)
		return (NULL); // consistency check

	sections_found = 0; // no sections found yet

	// for each entry in the dictionary...
	for (index = 0; index < Dictionary->entry_count; index++)
	{
		if (Dictionary->entries[index].key[0] == 0)
			continue; // skip empty slots

		// is this entry a section name ?
		if (wcschr (Dictionary->entries[index].key, INI_SECTION_SEPARATOR) == NULL)
		{
			// is it the section we want ?
			if (sections_found == section_index)
				return (Dictionary->entries[index].key); // then return its name

			sections_found++; // we found one section more
		}
	}

	return (INIFile_default_section); // section was not found, return the default section name
}


bool IniFile::ReadEntryAsBool (wchar_t *section, wchar_t *entry, bool default_value)
{
	// this function queries a dictionary for a key. A key as read from an ini file is given as
	// "section]key". If the key cannot be found, the notfound value is returned. A true boolean
	// is found if a string starting with either 'y', 'Y', 't', 'T' or '1' is matched. A false
	// boolean is found if a string starting with 'n', 'N', 'f', 'F' or '0' is matched.

	wchar_t *value;

	// get the value as a string first
	value = ReadEntryAsString ( section, entry, INI_INVALID_KEY);

	if (value == INI_INVALID_KEY)
		return (default_value); // if the entry could not be read, return the default value

	// decide of the boolean's value by looking at the first character
	if ((towupper (value[0]) == L'T') || (towupper (value[0]) == L'Y') || (value[0] == L'1'))
		return true; // boolean value is TRUE
	else if ((towupper (value[0]) == L'F') || (towupper (value[0]) == L'N') || (value[0] == L'0'))
		return false; // boolean value is FALSE

	return (default_value); // boolean value is undefined, return default value
}


long IniFile::ReadEntryAsLong (wchar_t *section, wchar_t *entry, long default_value)
{
	// this function queries a dictionary for a key. A key as read from an ini file is given as
	// "section]key". If the key cannot be found, the notfound value is returned.

	wchar_t *value;

	// get the value as a string first
	value = ReadEntryAsString ( section, entry, INI_INVALID_KEY);

	if (value == INI_INVALID_KEY)
		return (default_value); // if the entry could not be read, return the default value

	return (_wtoi (value)); // else convert the value to a long integer and return it
}


double IniFile::ReadEntryAsDouble (wchar_t *section, wchar_t *entry, double default_value)
{
	// this function queries a dictionary for a key. A key as read from an ini file is given as
	// "section]key". If the key cannot be found, the notfound value is returned.

	wchar_t *value;

	// get the value as a string first
	value = ReadEntryAsString ( section, entry, INI_INVALID_KEY);

	if (value == INI_INVALID_KEY)
		return (default_value); // if the entry could not be read, return the default value

	return (_wtof (value)); // else convert the value to a double precision number and return it
}


wchar_t* IniFile::ReadEntryAsString ( wchar_t *section, wchar_t *entry, wchar_t *default_value)
{
	// this function queries a dictionary for a key. A key as read from an ini file is given as
	// "section]key". If the key cannot be found, the pointer passed as 'def' is returned. The
	// returned char pointer is pointing to a string allocated in the dictionary, do not free
	// or modify it.

	int length;
	int i;
	wchar_t *value;
	wchar_t key[256];

	if (section == NULL)
		section = INIFile_default_section; // if no section was provided, use empty section name

	// if we were given an entry, build a key as section]entry
	if (entry != NULL)
	{
		swprintf(key, 256, L"%s%c%s", section, INI_SECTION_SEPARATOR, entry); // compose the key

		i = (int) wcslen (key); // get the key string length
		if (i < sizeof (key))
			length = i;
		else
			length = sizeof (key) - 1; // clamp it to a max value

		// for each character in the string after the section separator...
		for (i = (int) wcslen (section) + 1; i < length; i++)
			key[i] = towlower (key[i]); // convert it to lowercase
		key[i] = 0; // terminate the string
	}

	// else it must be a section name
	else
		wcscpy_s (key, 256, section); // copy the name into the key

	value = ReadKey ( key, default_value); // query the dictionary...
	return (value); // ...and return the value
}


void IniFile::WriteEntryAsBool (wchar_t *section, wchar_t *entry, bool value)
{
	// sets an entry in a dictionary. If the given entry can be found in the dictionary, it is
	// modified to contain the provided value. If it cannot be found, it is created.

	// according the boolean's value, write the equivalent string
	if (value)
		WriteEntryAsString ( section, entry, L"true"); // write the new entry
	else
		WriteEntryAsString ( section, entry, L"false"); // write the new entry

	return; // finished
}


void IniFile::WriteEntryAsLong (wchar_t *section, wchar_t *entry, long value)
{
	// sets an entry in a dictionary. If the given entry can be found in the dictionary, it is
	// modified to contain the provided value. If it cannot be found, it is created.

	// build the long integer value equivalent string (use printf facility)
	wchar_t value_as_string[256];
	swprintf (value_as_string, 256, L"%d", value);

	WriteEntryAsString ( section, entry, value_as_string); // write the new entry
	return; // finished
}


void IniFile::WriteEntryAsDouble ( wchar_t *section, wchar_t *entry, double value)
{
	// sets an entry in a dictionary. If the given entry can be found in the dictionary, it is
	// modified to contain the provided value. If it cannot be found, it is created.

	// build the long integer value equivalent string (use printf facility)
	wchar_t value_as_string[256];
	swprintf(value_as_string, 256, L"%g", value);

	WriteEntryAsString ( section, entry, value_as_string); // write the new entry
	return; // finished
}


void IniFile::WriteEntryAsString ( wchar_t *section, wchar_t *entry, wchar_t *value)
{
	// sets an entry in a dictionary. If the given entry can be found in the dictionary, it is
	// modified to contain the provided value. If it cannot be found, it is created.

	int length;
	int i;
	wchar_t key[256];

	if (section == NULL)
		section = INIFile_default_section; // if no section was provided, use empty section name

	// if we were given an entry, build a key as section#entry
	if (entry != NULL)
	{
		WriteKey ( section, NULL); // create the section if it doesn't exist

		swprintf (key, 256, L"%s%c%s", section, INI_SECTION_SEPARATOR, entry); // compose the key

		i = (int) wcslen (key); // get the key string length
		if (i < sizeof (key))
			length = i;
		else
			length = sizeof (key) - 1; // clamp it to a max value

		// for each character in the string after the section separator...
		for (i = (int) wcslen (section) + 1; i < length; i++)
			key[i] = towlower (key[i]); // convert it to lowercase
		key[i] = 0; // terminate the string
	}

	// else it must be a section name
	else
		wcscpy_s (key, 256, section); // copy the name into the key

	WriteKey ( key, value); // write the new key in the dictionary
	return; // finished
}


void IniFile::DeleteEntry ( wchar_t *section, wchar_t *entry)
{
	// deletes an entry in the dictionary

	int length;
	int i;
	wchar_t key[256];

	if (section == NULL)
		section = INIFile_default_section; // if no section was provided, use empty section name

	// if we were given an entry, build a key as section#entry
	if (entry != NULL)
	{
		swprintf (key, 256, L"%s%c%s", section, INI_SECTION_SEPARATOR, entry); // compose the key

		i = (int) wcslen (key); // get the key string length
		if (i < sizeof (key))
			length = i;
		else
			length = sizeof (key) - 1; // clamp it to a max value

		// for each character in the string after the section separator...
		for (i = (int) wcslen (section) + 1; i < length; i++)
			key[i] = towlower (key[i]); // convert it to lowercase
		key[i] = 0; // terminate the string
	}

	// else it must be a section name
	else
		wcscpy_s (key, 256, section); // copy the name into the key

	DeleteKey ( key);
	return;
}


void IniFile::DeleteSection ( wchar_t *section)
{
	// deletes a whole INI section in the dictionary

	int length;
	int i;
	wchar_t key[256];

	if (section == NULL)
		section = INIFile_default_section; // if no section was provided, use empty section name

	swprintf (key, 256, L"%s%c", section, INI_SECTION_SEPARATOR); // compose the key
	length = (int) wcslen (key); // get the key string length

	// for each entry in the dictionary...
	for (i = 0; i < Dictionary->entry_count; i++)
	{
		if (Dictionary->entries[i].key[0] == 0)
			continue; // skip empty slots

		// does this entry belong to the section we want ?
		if (wcsncmp (Dictionary->entries[i].key, key, length) == 0)
			DeleteKey ( Dictionary->entries[i].key); // yes, delete it
	}

	DeleteKey ( section); // and finally delete the section name itself
	return;
}


void IniFile::SaveFile (std::wstring& FileName)
{
	// this function dumps a given dictionary into a loadable ini file.

	FILE *fp;
	int section_index;
	wchar_t *section_name;
	int section_count;
	int index;
	int length;
	wchar_t key[256];

	// try to open the INI file in ASCII write mode
	fp = _wfsopen (FileName.c_str(), L"w", _SH_DENYNO);
	if (fp == NULL)
		return; // cancel if unable to open file

	// keep only the file name for the comment
	if (wcsrchr (FileName.c_str(), L'/') != NULL)
	{
		const wchar_t* Tmp= wcsrchr (FileName.c_str(), L'/') + 1;
		// print the INI file name as a comment
		fwprintf (fp, L"# %s\n", Tmp);
	}
	else if (wcsrchr (FileName.c_str(), L'\\') != NULL)
	{
		const wchar_t* Tmp = wcsrchr (FileName.c_str(), L'\\') + 1;
		// print the INI file name as a comment
		fwprintf (fp, L"# %s\n", Tmp);
	}


	// get the number of sections there are in this INI dictionary
	section_count = GetNumberOfSections ();

	// for each section...
	for (section_index = 0; section_index < section_count; section_index++)
	{
		section_name = GetSectionName (section_index); // read section name

		// is it the default section ?
		if (wcscmp (section_name, INIFile_default_section) == 0)
			fwprintf (fp, L"\n"); // don't put the default section's name in the INI file
		else
			fwprintf (fp, L"\n[%s]\n", section_name); // dump all other sections into the INI file

		// build the section identifier to be used when looking up the dictionary
		swprintf(key, 256, L"%s%c", section_name, INI_SECTION_SEPARATOR);
		length = (int) wcslen (key);

		// then for each entry in the dictionary...
		for (index = 0; index < Dictionary->entry_count; index++)
		{
			if (Dictionary->entries[index].key[0] == 0)
				continue; // skip empty slots

			// does this key belong to the section we want ? if so, dump it
			if (wcsncmp (Dictionary->entries[index].key, key, length) == 0)
			{
				// if key starts or ends with a space, enclose it between quotes, else don't
				if ((Dictionary->entries[index].value[0] != 0)
					&& (iswspace (Dictionary->entries[index].value[0])
					|| iswspace (Dictionary->entries[index].value[wcslen (Dictionary->entries[index].value) - 1])))
					fwprintf (fp, L"%s = \"%s\"\n", Dictionary->entries[index].key + length, Dictionary->entries[index].value);
				else
					fwprintf (fp, L"%s = %s\n", Dictionary->entries[index].key + length, Dictionary->entries[index].value);
			}
		}
	}

	fclose (fp); // finished, close the file

	return; // and return
}
