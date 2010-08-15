// inifile.c

#include "client.h"


// WARNING: INI SECTION NAMES ARE CASE SENSITIVE, BUT KEY NAMES ARE NOT!


// internal definitions
#define DICTIONARY_PAGE_SIZE 128
#define INI_SECTION_SEPARATOR ']'
#define INI_INVALID_KEY (char *) 0x7fffffff


// dictionary structure definitions
typedef struct dictionary_entry_s
{
   char key[126]; // key string
   unsigned long hash; // key hash value
   char value[256]; // value string
} dictionary_entry_t;


typedef struct dictionary_s
{
   int size; // storage size
   dictionary_entry_t *entries; // array of entries (mallocated)
   int entry_count; // number of entries in dictionary
} dictionary_t;


// dictionary function prototypes
static dictionary_t *Dictionary_CreateDictionary (int size);
static void Dictionary_DestroyDictionary (dictionary_t *dictionary);
static dictionary_entry_t *Dictionary_GetKey (dictionary_t *dictionary, char *key);
static char *Dictionary_ReadKey (dictionary_t *dictionary, char *key, char *default_value);
static void Dictionary_WriteKey (dictionary_t *dictionary, char *key, char *value);
static void Dictionary_DeleteKey (dictionary_t *dictionary, char *key);
static unsigned long Dictionary_ComputeHashValueForKey (char *key);


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


// internal variables
static char *INIFile_default_section = "__default";
static char line_buffer[1024];
static char key[256];
static char value_as_string[256];
static char section[256];
static char entry[256];
static char value[256];


static dictionary_t *Dictionary_CreateDictionary (int size)
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


static void Dictionary_DestroyDictionary (dictionary_t *dictionary)
{
   // frees a dictionary object and all memory associated to it.

   if (dictionary == NULL)
      return; // consistency check

   free (dictionary->entries); // free the entries array
   free (dictionary); // and finally, free the dictionary itself

   return; // finished
}


static dictionary_entry_t *Dictionary_GetKey (dictionary_t *dictionary, char *key)
{
   // this function locates a key in a dictionary and returns a pointer to it, or a NULL
   // pointer if no such key can be found in dictionary. The returned pointer points to data
   // internal to the dictionary object, do not try to free or modify it.

   unsigned long hash;
   register int index;

   hash = Dictionary_ComputeHashValueForKey (key); // get the hash value for key

   // cycle through all entries in the dictionary
   for (index = 0; index < dictionary->entry_count; index++)
   {
      if (dictionary->entries[index].key[0] == 0)
         continue; // skip empty slots

      // compare hash AND string, to avoid hash collisions
      if ((hash == dictionary->entries[index].hash) && (strcmp (key, dictionary->entries[index].key) == 0))
         return (&dictionary->entries[index]); // return a pointer to the key if we find it
   }

   return (NULL); // else return a NULL pointer
}


static char *Dictionary_ReadKey (dictionary_t *dictionary, char *key, char *default_value)
{
   // this function locates a key in a dictionary and returns a pointer to its value, or the
   // passed 'def' pointer if no such key can be found in dictionary. The returned char pointer
   // points to data internal to the dictionary object, do not try to free or modify it.

   dictionary_entry_t *element;

   element = Dictionary_GetKey (dictionary, key); // query the dictionary for the key
   if (element != NULL)
      return (element->value); // if we found a valid key, return the value associed to it

   return (default_value); // else return the default value
}


static void Dictionary_WriteKey (dictionary_t *dictionary, char *key, char *value)
{
   // sets a value in a dictionary. If the given key is found in the dictionary, the associated
   // value is replaced by the provided one. If the key cannot be found in the dictionary, it
   // is added to it.

   unsigned long hash;
   register int index;

   hash = Dictionary_ComputeHashValueForKey (key); // compute hash for this key

   // for each entry in the dictionary...
   for (index = 0; index < dictionary->entry_count; index++)
   {
      if (dictionary->entries[index].key[0] == 0)
          continue; // skip empty slots

      // does that key have the same hash value AND the same name ?
      if ((hash == dictionary->entries[index].hash) && (strcmp (key, dictionary->entries[index].key) == 0))
      {
         // we've found the right key: modify its value and return

         // have we provided a valid value ?
         if (value != NULL)
            strcpy_s (dictionary->entries[index].value, sizeof (dictionary->entries[index].value), value); // copy the value string
         else
            dictionary->entries[index].value[0] = 0; // reset the value string

         return; // value has been modified: return
      }
   }

   // here either the dictionary was empty, or we couldn't find a similar value: add a new one

   // cycle through all entries in the dictionary...
   for (index = 0; index < dictionary->entry_count; index++)
      if (dictionary->entries[index].key[0] == 0)
         break; // and stop at the first empty one we find

   // no empty entry found, see if dictionary needs to grow or if there's still room left
   if (index == dictionary->size)
   {
      // mallocate some more space for the dictionary and zero all that crap out
      dictionary->entries = (dictionary_entry_t *) realloc (dictionary->entries, (dictionary->size + DICTIONARY_PAGE_SIZE) * sizeof (dictionary_entry_t)); // alloc 1 more page
      dictionary->size += DICTIONARY_PAGE_SIZE; // increase dictionary size
   }

   // copy the new key

   strcpy_s (dictionary->entries[index].key, sizeof (dictionary->entries[index].key), key); // copy the key string...
   dictionary->entries[index].hash = hash; // ...and store the hash value

   // have we provided a valid value ?
   if (value != NULL)
      strcpy_s (dictionary->entries[index].value, sizeof (dictionary->entries[index].value), value); // copy the value string
   else
      dictionary->entries[index].value[0] = 0; // reset the value string

   dictionary->entry_count++; // there is one more entry in the dictionary now
   return;
}


static void Dictionary_DeleteKey (dictionary_t *dictionary, char *key)
{
   // this function deletes a key in a dictionary. Nothing is done if the key cannot be found.

   unsigned long hash;
   register int index;

   hash = Dictionary_ComputeHashValueForKey (key); // get the hash value for key

   // cycle through all entries in the dictionary...
   for (index = 0; index < dictionary->entry_count; index++)
   {
      if (dictionary->entries[index].key[0] == 0)
         continue; // skip empty slots

      // compare hash AND string, to avoid hash collisions
      if ((hash == dictionary->entries[index].hash) && (strcmp (key, dictionary->entries[index].key) == 0))
         break; // break as soon as we've found the key we want
   }

   if (index == dictionary->entry_count)
      return; // if key was not found, just return

   // clear the key, the hash and its value data
   memset (&dictionary->entries[index], 0, sizeof (dictionary_entry_t));

   dictionary->entry_count--; // there is one entry less in the dictionary now
   return;
}


static unsigned long Dictionary_ComputeHashValueForKey (char *key)
{
   // Compute the hash key for a string. This hash function has been taken from an article in
   // Dr Dobbs Journal. This is normally a collision-free function, distributing keys evenly.
   // The key is stored anyway in the struct so that collision can be avoided by comparing the
   // key itself in last resort. THIS FUNCTION IS TO BE USED INTERNALLY ONLY!

   register unsigned long hash;
   int length;
   int index;

   hash = 0;
   length = (int) strlen (key);

   // for each character of the string...
   for (index = 0; index < length; index++)
   {
      hash += (unsigned long) key[index]; // take it in account and compute the hash value
      hash += (hash << 10);
      hash ^= (hash >> 6);
   }

   hash += (hash << 3); // finalize hashing (what the hell does it do, I have no clue)
   hash ^= (hash >> 11);
   hash += (hash << 15);

   return (hash); // and return the hash value
}


void *INIFile_NewINIFile (void)
{
   // allocates a dictionary for a new INI file. Mostly a helper function.

   return ((void *) Dictionary_CreateDictionary (0));
}


int INIFile_GetNumberOfSections (void  *ini_data)
{
   // this function returns the number of sections found in a dictionary. The test to recognize
   // sections is done on the string stored in the dictionary: a section name is given as
   // "section" whereas a key is stored as "section]key", thus the test looks for entries that
   // do NOT contain a ']'. This clearly fails in the case a section name contains a ']',
   // but this should simply be avoided.

   int section_count;
   int index;
   dictionary_t *dictionary;

   if (ini_data == NULL)
      return (0); // consistency check

   dictionary = (dictionary_t *) ini_data;

   section_count = 0; // no sections found yet

   // for each entry in the dictionary...
   for (index = 0; index < dictionary->entry_count; index++)
   {
      if (dictionary->entries[index].key[0] == 0)
         continue; // skip empty slots

      if (strchr (dictionary->entries[index].key, INI_SECTION_SEPARATOR) == NULL)
         section_count++; // if this entry has NO section separator, then it's a section name
   }

   return (section_count); // return the section count we found
}


char *INIFile_GetSectionName (void *ini_data, int section_index)
{
   // this function locates the n-th section in a dictionary and returns its name as a pointer
   // to a string allocated inside the dictionary. Do not free or modify the returned string!!
   // This function returns NULL in case of error.

   int sections_found;
   int index;
   dictionary_t *dictionary;

   if ((ini_data == NULL) || (section_index < 0))
      return (NULL); // consistency check

   dictionary = (dictionary_t *) ini_data;

   sections_found = 0; // no sections found yet

   // for each entry in the dictionary...
   for (index = 0; index < dictionary->entry_count; index++)
   {
      if (dictionary->entries[index].key[0] == 0)
         continue; // skip empty slots

      // is this entry a section name ?
      if (strchr (dictionary->entries[index].key, INI_SECTION_SEPARATOR) == NULL)
      {
         // is it the section we want ?
         if (sections_found == section_index)
            return (dictionary->entries[index].key); // then return its name

         sections_found++; // we found one section more
      }
   }

   return (INIFile_default_section); // section was not found, return the default section name
}


unsigned char INIFile_ReadEntryAsBool (void *ini_data, char *section, char *entry, unsigned char default_value)
{
   // this function queries a dictionary for a key. A key as read from an ini file is given as
   // "section]key". If the key cannot be found, the notfound value is returned. A true boolean
   // is found if a string starting with either 'y', 'Y', 't', 'T' or '1' is matched. A false
   // boolean is found if a string starting with 'n', 'N', 'f', 'F' or '0' is matched.

   char *value;

   if (ini_data == NULL)
      return (default_value); // consistency check

   // get the value as a string first
   value = INIFile_ReadEntryAsString ((dictionary_t *) ini_data, section, entry, INI_INVALID_KEY);

   if (value == INI_INVALID_KEY)
      return (default_value); // if the entry could not be read, return the default value

   // decide of the boolean's value by looking at the first character
   if ((toupper (value[0]) == 'T') || (toupper (value[0]) == 'Y') || (value[0] == '1'))
     return (1); // boolean value is TRUE
   else if ((toupper (value[0]) == 'F') || (toupper (value[0]) == 'N') || (value[0] == '0'))
     return (0); // boolean value is FALSE

   return (default_value); // boolean value is undefined, return default value
}


long INIFile_ReadEntryAsLong (void *ini_data, char *section, char *entry, long default_value)
{
   // this function queries a dictionary for a key. A key as read from an ini file is given as
   // "section]key". If the key cannot be found, the notfound value is returned.

   char *value;

   if (ini_data == NULL)
      return (default_value); // consistency check

   // get the value as a string first
   value = INIFile_ReadEntryAsString ((dictionary_t *) ini_data, section, entry, INI_INVALID_KEY);

   if (value == INI_INVALID_KEY)
      return (default_value); // if the entry could not be read, return the default value

   return (atoi (value)); // else convert the value to a long integer and return it
}


double INIFile_ReadEntryAsDouble (void *ini_data, char *section, char *entry, double default_value)
{
   // this function queries a dictionary for a key. A key as read from an ini file is given as
   // "section]key". If the key cannot be found, the notfound value is returned.

   char *value;

   if (ini_data == NULL)
      return (default_value); // consistency check

   // get the value as a string first
   value = INIFile_ReadEntryAsString ((dictionary_t *) ini_data, section, entry, INI_INVALID_KEY);

   if (value == INI_INVALID_KEY)
      return (default_value); // if the entry could not be read, return the default value

   return (atof (value)); // else convert the value to a double precision number and return it
}


char *INIFile_ReadEntryAsString (void *ini_data, char *section, char *entry, char *default_value)
{
   // this function queries a dictionary for a key. A key as read from an ini file is given as
   // "section]key". If the key cannot be found, the pointer passed as 'def' is returned. The
   // returned char pointer is pointing to a string allocated in the dictionary, do not free
   // or modify it.

   int length;
   int i;
   char *value;

   if (ini_data == NULL)
      return (default_value); // consistency check

   if (section == NULL)
      section = INIFile_default_section; // if no section was provided, use empty section name

   // if we were given an entry, build a key as section]entry
   if (entry != NULL)
   {
      sprintf_s (key, sizeof (key), "%s%c%s", section, INI_SECTION_SEPARATOR, entry); // compose the key

      i = (int) strlen (key); // get the key string length
      if (i < sizeof (key))
         length = i;
      else
         length = sizeof (key) - 1; // clamp it to a max value

      // for each character in the string after the section separator...
      for (i = (int) strlen (section) + 1; i < length; i++)
         key[i] = tolower (key[i]); // convert it to lowercase
      key[i] = 0; // terminate the string
   }

   // else it must be a section name
   else
      strcpy_s (key, sizeof (key), section); // copy the name into the key

   value = Dictionary_ReadKey ((dictionary_t *) ini_data, key, default_value); // query the dictionary...
   return (value); // ...and return the value
}


void INIFile_WriteEntryAsBool (void *ini_data, char *section, char *entry, unsigned char value)
{
   // sets an entry in a dictionary. If the given entry can be found in the dictionary, it is
   // modified to contain the provided value. If it cannot be found, it is created.

   // according the boolean's value, write the equivalent string
   if (value)
      INIFile_WriteEntryAsString ((dictionary_t *) ini_data, section, entry, "true"); // write the new entry
   else
      INIFile_WriteEntryAsString ((dictionary_t *) ini_data, section, entry, "false"); // write the new entry

   return; // finished
}


void INIFile_WriteEntryAsLong (void *ini_data, char *section, char *entry, long value)
{
   // sets an entry in a dictionary. If the given entry can be found in the dictionary, it is
   // modified to contain the provided value. If it cannot be found, it is created.

   // build the long integer value equivalent string (use printf facility)
   sprintf_s (value_as_string, sizeof (value_as_string), "%d", value);

   INIFile_WriteEntryAsString ((dictionary_t *) ini_data, section, entry, value_as_string); // write the new entry
   return; // finished
}


void INIFile_WriteEntryAsDouble (void *ini_data, char *section, char *entry, double value)
{
   // sets an entry in a dictionary. If the given entry can be found in the dictionary, it is
   // modified to contain the provided value. If it cannot be found, it is created.

   // build the long integer value equivalent string (use printf facility)
   sprintf_s (value_as_string, sizeof (value_as_string), "%g", value);

   INIFile_WriteEntryAsString ((dictionary_t *) ini_data, section, entry, value_as_string); // write the new entry
   return; // finished
}


void INIFile_WriteEntryAsString (void *ini_data, char *section, char *entry, char *value)
{
   // sets an entry in a dictionary. If the given entry can be found in the dictionary, it is
   // modified to contain the provided value. If it cannot be found, it is created.

   int length;
   int i;

   if (ini_data == NULL)
      return; // consistency check

   if (section == NULL)
      section = INIFile_default_section; // if no section was provided, use empty section name

   // if we were given an entry, build a key as section#entry
   if (entry != NULL)
   {
      Dictionary_WriteKey ((dictionary_t *) ini_data, section, NULL); // create the section if it doesn't exist

      sprintf_s (key, sizeof (key), "%s%c%s", section, INI_SECTION_SEPARATOR, entry); // compose the key

      i = (int) strlen (key); // get the key string length
      if (i < sizeof (key))
         length = i;
      else
         length = sizeof (key) - 1; // clamp it to a max value

      // for each character in the string after the section separator...
      for (i = (int) strlen (section) + 1; i < length; i++)
         key[i] = tolower (key[i]); // convert it to lowercase
      key[i] = 0; // terminate the string
   }

   // else it must be a section name
   else
      strcpy_s (key, sizeof (key), section); // copy the name into the key

   Dictionary_WriteKey ((dictionary_t *) ini_data, key, value); // write the new key in the dictionary
   return; // finished
}


void INIFile_DeleteEntry (void *ini_data, char *section, char *entry)
{
   // deletes an entry in the dictionary

   int length;
   int i;

   if (ini_data == NULL)
      return; // consistency check

   if (section == NULL)
      section = INIFile_default_section; // if no section was provided, use empty section name

   // if we were given an entry, build a key as section#entry
   if (entry != NULL)
   {
      sprintf_s (key, sizeof (key), "%s%c%s", section, INI_SECTION_SEPARATOR, entry); // compose the key

      i = (int) strlen (key); // get the key string length
      if (i < sizeof (key))
         length = i;
      else
         length = sizeof (key) - 1; // clamp it to a max value

      // for each character in the string after the section separator...
      for (i = (int) strlen (section) + 1; i < length; i++)
         key[i] = tolower (key[i]); // convert it to lowercase
      key[i] = 0; // terminate the string
   }

   // else it must be a section name
   else
      strcpy_s (key, sizeof (key), section); // copy the name into the key

   Dictionary_DeleteKey ((dictionary_t *) ini_data, key);
   return;
}


void INIFile_DeleteSection (void *ini_data, char *section)
{
   // deletes a whole INI section in the dictionary

   int length;
   int i;
   dictionary_t *dictionary;

   if (ini_data == NULL)
      return; // consistency check

   if (section == NULL)
      section = INIFile_default_section; // if no section was provided, use empty section name

   dictionary = (dictionary_t *) ini_data;

   sprintf_s (key, sizeof (key), "%s%c", section, INI_SECTION_SEPARATOR); // compose the key
   length = (int) strlen (key); // get the key string length

   // for each entry in the dictionary...
   for (i = 0; i < dictionary->entry_count; i++)
   {
      if (dictionary->entries[i].key[0] == 0)
         continue; // skip empty slots

      // does this entry belong to the section we want ?
      if (strncmp (dictionary->entries[i].key, key, length) == 0)
         Dictionary_DeleteKey (dictionary, dictionary->entries[i].key); // yes, delete it
   }

   Dictionary_DeleteKey (dictionary, section); // and finally delete the section name itself
   return;
}


void *INIFile_LoadINIFile (const char *filename)
{
   // this is the parser for ini files. This function is called, providing the name of the file
   // to be read. It returns a dictionary object that should not be accessed directly, but
   // through accessor functions instead.

   FILE *fp;
   int fieldstart;
   int fieldstop;
   int length;
   int i;
   dictionary_t *dictionary;

   // try to open the INI file in ASCII read-only mode
   fp = _fsopen (filename, "r", _SH_DENYNO);
   if (fp == NULL)
      return (NULL); // cancel if file not found

   dictionary = Dictionary_CreateDictionary (0);

   // set the default section for orphaned entries and add it to the dictionary
   strcpy_s (section, sizeof (section), INIFile_default_section);
   INIFile_WriteEntryAsString (dictionary, section, NULL, NULL);

   // read line per line...
   while (fgets (line_buffer, 1024, fp) != NULL)
   {
      length = (int) strlen (line_buffer); // get line length

      while ((length > 0) && ((line_buffer[length - 1] == '\n') || (line_buffer[length - 1] == '\r')))
         length--; // discard trailing newlines

      fieldstart = 0; // let's now strip leading blanks
      while ((fieldstart < length) && isspace ((unsigned char) line_buffer[fieldstart]))
         fieldstart++; // ignore any tabs or spaces, going forward from the start

      fieldstop = length - 1; // let's now strip trailing blanks
      while ((fieldstop >= 0) && isspace ((unsigned char) line_buffer[fieldstop]))
         fieldstop--; // ignore any tabs or spaces, going backwards from the end

      for (i = fieldstart; i <= fieldstop; i++)
         line_buffer[i - fieldstart] = line_buffer[i]; // recopy line buffer without the spaces
      line_buffer[i - fieldstart] = 0; // and terminate the string

      if ((line_buffer[0] == ';') || (line_buffer[0] == '#') || (line_buffer[0] == 0))
         continue; // skip comment lines

      // is it a valid section name ?
      if (sscanf_s (line_buffer, "[%[^]]", section, sizeof (section)) == 1)
      {
         length = (int) strlen (section); // get the section string length

         fieldstart = 0; // let's now strip leading blanks
         while ((fieldstart < length) && isspace ((unsigned char) section[fieldstart]))
            fieldstart++; // ignore any tabs or spaces, going forward from the start

         fieldstop = length - 1; // let's now strip trailing blanks
         while ((fieldstop >= 0) && isspace ((unsigned char) section[fieldstop]))
            fieldstop--; // ignore any tabs or spaces, going backwards from the end

         for (i = fieldstart; i <= fieldstop; i++)
            section[i - fieldstart] = section[i]; // recopy section name w/out spaces
         section[i - fieldstart] = 0; // and terminate the string

         INIFile_WriteEntryAsString (dictionary, section, NULL, NULL); // add to dictionary
      }

      // else is it a valid entry/value pair that is enclosed between quotes?
      else if (sscanf_s (line_buffer, "%[^=] = \"%[^\"]\"", entry, sizeof (entry), value, sizeof (value)) == 2)
      {
         length = (int) strlen (entry); // get the entry string length

         fieldstart = 0; // let's now strip leading blanks
         while ((fieldstart < length) && isspace ((unsigned char) entry[fieldstart]))
            fieldstart++; // ignore any tabs or spaces, going forward from the start

         fieldstop = length - 1; // let's now strip trailing blanks
         while ((fieldstop >= 0) && isspace ((unsigned char) entry[fieldstop]))
            fieldstop--; // ignore any tabs or spaces, going backwards from the end

         for (i = fieldstart; i <= fieldstop; i++)
            entry[i - fieldstart] = tolower (entry[i]); // recopy entry name w/out spaces
         entry[i - fieldstart] = 0; // and terminate the string

         // when value is enclosed between quotes, DO NOT strip the blanks

         // sscanf cannot handle "" or '' as empty value, this is done here
         if ((strcmp (value, "\"\"") == 0) || (strcmp (value, "''") == 0))
            value[0] = 0; // empty string

         INIFile_WriteEntryAsString (dictionary, section, entry, value); // add to dictionary
      }

      // else is it a valid entry/value pair without quotes ?
      else if ((sscanf_s (line_buffer, "%[^=] = '%[^\']'", entry, sizeof (entry), value, sizeof (value)) == 2)
               || (sscanf_s (line_buffer, "%[^=] = %[^;#]", entry, sizeof (entry), value, sizeof (value)) == 2))
      {
         length = (int) strlen (entry); // get the entry string length

         fieldstart = 0; // let's now strip leading blanks
         while ((fieldstart < length) && isspace ((unsigned char) entry[fieldstart]))
            fieldstart++; // ignore any tabs or spaces, going forward from the start

         fieldstop = length - 1; // let's now strip trailing blanks
         while ((fieldstop >= 0) && isspace ((unsigned char) entry[fieldstop]))
            fieldstop--; // ignore any tabs or spaces, going backwards from the end

         for (i = fieldstart; i <= fieldstop; i++)
            entry[i - fieldstart] = tolower (entry[i]); // recopy entry name w/out spaces
         entry[i - fieldstart] = 0; // and terminate the string

         length = (int) strlen (value); // get the value string length

         fieldstart = 0; // let's now strip leading blanks
         while ((fieldstart < length) && isspace ((unsigned char) value[fieldstart]))
            fieldstart++; // ignore any tabs or spaces, going forward from the start

         fieldstop = length - 1; // let's now strip trailing blanks
         while ((fieldstop >= 0) && isspace ((unsigned char) value[fieldstop]))
            fieldstop--; // ignore any tabs or spaces, going backwards from the end

         for (i = fieldstart; i <= fieldstop; i++)
            value[i - fieldstart] = value[i]; // recopy entry name w/out spaces
         value[i - fieldstart] = 0; // and terminate the string

         // sscanf cannot handle "" or '' as empty value, this is done here
         if ((strcmp (value, "\"\"") == 0) || (strcmp (value, "''") == 0))
            value[0] = 0; // empty string

         INIFile_WriteEntryAsString (dictionary, section, entry, value); // add to dictionary
      }
   }

   fclose (fp); // finished, close the file

   return ((void *) dictionary); // and return
}


void INIFile_SaveINIFile (const char *filename, void *ini_data)
{
   // this function dumps a given dictionary into a loadable ini file.

   FILE *fp;
   int section_index;
   char *section_name;
   int section_count;
   int index;
   int length;
   dictionary_t *dictionary;

   // try to open the INI file in ASCII write mode
   fp = _fsopen (filename, "w", _SH_DENYNO);
   if (fp == NULL)
      return; // cancel if unable to open file

   // get a hand on the INI data dictionary
   dictionary = (dictionary_t *) ini_data;

   // keep only the file name for the comment
   if (strrchr (filename, '/') != NULL)
      filename = strrchr (filename, '/') + 1;
   else if (strrchr (filename, '\\') != NULL)
      filename = strrchr (filename, '\\') + 1;

   // print the INI file name as a comment
   fprintf (fp, "# %s\n", filename);

   // get the number of sections there are in this INI dictionary
   section_count = INIFile_GetNumberOfSections (dictionary);

   // for each section...
   for (section_index = 0; section_index < section_count; section_index++)
   {
      section_name = INIFile_GetSectionName (dictionary, section_index); // read section name

      // is it the default section ?
      if (strcmp (section_name, INIFile_default_section) == 0)
         fprintf (fp, "\n"); // don't put the default section's name in the INI file
      else
         fprintf (fp, "\n[%s]\n", section_name); // dump all other sections into the INI file

      // build the section identifier to be used when looking up the dictionary
      sprintf_s (key, sizeof (key), "%s%c", section_name, INI_SECTION_SEPARATOR);
      length = (int) strlen (key);

      // then for each entry in the dictionary...
      for (index = 0; index < dictionary->entry_count; index++)
      {
         if (dictionary->entries[index].key[0] == 0)
            continue; // skip empty slots

         // does this key belong to the section we want ? if so, dump it
         if (strncmp (dictionary->entries[index].key, key, length) == 0)
         {
            // if key starts or ends with a space, enclose it between quotes, else don't
            if ((dictionary->entries[index].value[0] != 0)
                && (isspace (dictionary->entries[index].value[0])
                    || isspace (dictionary->entries[index].value[strlen (dictionary->entries[index].value) - 1])))
               fprintf (fp, "%s = \"%s\"\n", dictionary->entries[index].key + length, dictionary->entries[index].value);
            else
               fprintf (fp, "%s = %s\n", dictionary->entries[index].key + length, dictionary->entries[index].value);
         }
      }
   }

   fclose (fp); // finished, close the file

   return; // and return
}


void INIFile_FreeINIFile (void *ini_data)
{
   // this function frees the dictionary object used to store an INI file data

   if (ini_data == NULL)
      return; // consistency check

   Dictionary_DestroyDictionary ((dictionary_t *) ini_data); // just destroy the dictionary
   return; // finished
}
