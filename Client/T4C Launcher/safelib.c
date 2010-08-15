// safelib.c

#include "client.h"


// enable this library to be used with other projects
#ifndef T4CGUARD_SERVER
#ifndef T4CGUARD_CLIENT
#ifndef T4CGUARD_PLUGIN
#define Log_Text fprintf
#undef LOG_MAIN
#define LOG_MAIN stderr
#endif
#endif
#endif


// malloc debug macro definition
//#define DEBUG_MALLOC


void *SAFE_realloc (void *allocation, int old_count, int new_count, size_t element_size, bool cleanup)
{
   // handy wrapper for things we always forget, like checking malloc's returned pointer and
   // ensuring the extra allocated space is zeroed out if necessary.

   void *new_allocation;

   // do we want to reallocate a void space ?
   if (new_count == 0)
   {
      SAFE_free (&allocation); // free it if needed
      return (NULL); // and return an empty pointer
   }

   // do we NOT actually need to reallocate ?
   if (old_count == new_count)
      return (allocation); // allocation is well-suited for the purpose already

   // try re allocating the requested size
   new_allocation = realloc (allocation, new_count * element_size);

#ifdef DEBUG_MALLOC
   // on debug mode, print out verbose info
   Log_Text (LOG_MAIN, "SAFE_realloc(): Reallocating (%d -> %d blocks) of %d bytes each (total: %d bytes) from 0x%x to 0x%x\n", old_count, new_count, element_size, new_count * element_size, allocation, new_allocation);
#endif

   if (new_allocation == NULL)
   {
      Log_Text (LOG_MAIN, "SAFE_realloc(): failure reallocating from %d to %d blocks of %d bytes at 0x%x!\n", old_count, new_count, element_size, allocation);
      return (new_allocation);
   }

   // zero out extra allocated content if necessary
   if (cleanup && (new_count > old_count))
      memset ((void *) ((unsigned long) new_allocation + (old_count * element_size)), 0, (new_count - old_count) * element_size);

   return (new_allocation); // here we are guaranteed to have some correctly reallocated space
}


void SAFE_free (void **address_of_pointer_to_allocation)
{
   // handy wrapper for things we always forget, like checking the buffer pointer's validity
   // before freeing it and nulling it out after it has been freed.

   if (address_of_pointer_to_allocation == NULL)
      return; // consistency check

#ifdef DEBUG_MALLOC
   // on debug mode, print out verbose info
   Log_Text (LOG_MAIN, "SAFE_free(): Freeing blocks at 0x%x\n", *address_of_pointer_to_allocation);
#endif

   // is it a valid block pointer ?
   if (*address_of_pointer_to_allocation != NULL)
      free (*address_of_pointer_to_allocation); // only free valid block pointers
   *address_of_pointer_to_allocation = NULL; // reset the pointer (ensures it won't get freed twice)

   return; // finished
}


void *SAFE_malloc (int count, size_t element_size, bool cleanup)
{
   // handy wrapper for things we always forget, like checking malloc's returned pointer and
   // ensuring the allocated space is zeroed out.

   // here we are guaranteed to have some correctly allocated space
   return (SAFE_realloc (NULL, 0, count, element_size, cleanup));
}


char *SAFE_strncpy (char *destination, const char *source, size_t buffer_size)
{
   // handy wrapper for operations we always forget, like checking for string terminations

   unsigned int index;

   // first off, check if buffer size is valid
   if (buffer_size <= 0)
   {
      Log_Text (LOG_MAIN, "SAFE_strncpy(): function called with invalid buffer size!\n");
      return (destination);
   }

   // also check if the destination buffer is valid
   if (destination == NULL)
   {
      Log_Text (LOG_MAIN, "SAFE_strncpy(): function called with invalid destination buffer!\n");
      return (destination);
   }

   // for each character we find up to buffer_size...
   for (index = 0; index < buffer_size; index++)
   {
      destination[index] = source[index]; // copy everything
      if (source[index] == 0)
         break; // don't copy beyond the end of source
   }
   destination[buffer_size - 1] = 0; // ...but let us set the terminator zero ourselves

   return (destination); // finished
}


int SAFE_snprintf (char *destination, size_t buffer_size, const char *fmt, ...)
{
   // handy wrapper for operations we always forget, like checking for string terminations.
   // This function should return the number of characters that would be written if there had
   // been enough buffer space to be C99 compatible.

   va_list argptr;
   int result;

   // first off, check if buffer size is valid
   if (buffer_size <= 0)
   {
      Log_Text (LOG_MAIN, "SAFE_snprintf(): function called with invalid buffer size!\n");
      return (0);
   }

   // also check if the destination buffer is valid
   if (destination == NULL)
   {
      Log_Text (LOG_MAIN, "SAFE_snprintf(): function called with invalid destination buffer!\n");
      return (0);
   }

   // concatenate all the arguments in the destination string...
   va_start (argptr, fmt);
   result = vsnprintf_s (destination, buffer_size, buffer_size, fmt, argptr);
   va_end (argptr);

   if (result < 0)
      destination[buffer_size - 1] = 0; // ...but let us set the terminator zero ourselves

   return (result); // finished
}


int SAFE_vsnprintf (char *destination, size_t buffer_size, const char *fmt, va_list argptr)
{
   // handy wrapper for operations we always forget, like checking for string terminations.
   // This function should return the number of characters that would be written if there had
   // been enough buffer space to be C99 compatible.

   int result;

   // first off, check if buffer size is valid
   if (buffer_size <= 0)
   {
      Log_Text (LOG_MAIN, "SAFE_vsnprintf(): function called with invalid buffer size!\n");
      return (0);
   }

   // also check if the destination buffer is valid
   if (destination == NULL)
   {
      Log_Text (LOG_MAIN, "SAFE_vsnprintf(): function called with invalid destination buffer!\n");
      return (0);
   }

   // concatenate all the arguments in the destination string...
   result = vsnprintf_s (destination, buffer_size, buffer_size, fmt, argptr);
   if (result < 0)
      destination[buffer_size - 1] = 0; // ...but let us set the terminator zero ourselves

   return (result); // finished
}


char *SAFE_strncatf (char *destination, size_t buffer_size, const char *fmt, ...)
{
   // handy wrapper for operations we always forget, like checking for string terminations.
   // This function adds variable arguments to strcat(), printf()-style.

   va_list argptr;
   char message[1024];
   int index;
   int result;
   unsigned int index2;
   unsigned int amount_to_copy;

   // first off, check if buffer size is valid
   if (buffer_size <= 0)
   {
      Log_Text (LOG_MAIN, "SAFE_strncatf(): function called with invalid buffer size!\n");
      return (destination);
   }

   // also check if the destination buffer is valid
   if (destination == NULL)
   {
      Log_Text (LOG_MAIN, "SAFE_strncatf(): function called with invalid destination buffer!\n");
      return (destination);
   }

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   result = vsnprintf_s (message, sizeof (message), sizeof (message), fmt, argptr);
   va_end (argptr);
//   if (result < 0)
      message[sizeof (message) - 1] = 0; // ...but let us set the terminator zero ourselves

   // get the destination string end position
   index = strlen (destination);

   // see how much data we can copy
   amount_to_copy = strlen (message);
   if (amount_to_copy > buffer_size - index)
      amount_to_copy = buffer_size - index; // don't copy more than the space left

   // and finally append ourselves the right number of characters, not to overflow
   for (index2 = 0; index2 < amount_to_copy; index2++)
   {
      destination[index + index2] = message[index2]; // append message to destination string
      if (message[index2] == 0)
         break; // don't copy beyond the end of source
   }
   destination[index + amount_to_copy] = 0; // ...but let us set the terminator zero ourselves

   return (destination); // finished
}


void SAFE_fclose (FILE **address_of_pointer_to_file)
{
   // handy wrapper for things we always forget, like checking the file pointer's validity
   // before freeing it and nulling it out after it has been freed.

   // is it a valid file pointer ?
   if (*address_of_pointer_to_file != NULL)
   {
      fclose (*address_of_pointer_to_file); // only free valid file pointers
      *address_of_pointer_to_file = NULL; // don't forget to reset the pointer (this ensures it won't get freed twice)
   }

   return; // finished
}
