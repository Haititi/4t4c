// c99.h

#ifndef C99_H
#define C99_H

// stuff to ensure compilers comply to the ANSI C99 standard

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>


#ifndef access
#define access _access
#endif

#ifndef W_OK
#define W_OK 0x02
#endif

#ifndef R_OK
#define R_OK 0x04
#endif

#ifndef snprintf
#define snprintf _snprintf
#endif

#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif

#ifndef ioctl
#define ioctl ioctlsocket
#endif

#ifndef stricmp
#define stricmp _stricmp
#endif

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif

#ifndef strnicmp
#define strnicmp _strnicmp
#endif

#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

#ifndef strcasestr
#define strcasestr stristr
#endif

#ifndef mkdir
#define mkdir _mkdir
#endif

#ifndef unlink
#define unlink _unlink
#endif

#ifndef spawnl
#define spawnl _spawnl
#endif

#ifndef stristr
static char *stristr (const char *haystack, const char *needle)
{
   // windows has no stristr() implementation, so here is mine.
   const char *ptr_upper, *ptr_lower, *ptr_either;
   size_t needle_length;
   needle_length = strlen (needle); // get needle length
   ptr_either = haystack; // start searching at the beginning of haystack
   for (;;) // endless loop
   {
      ptr_upper = strchr (haystack, toupper (*needle)); // find occurence of first character (uppercase)
      ptr_lower = strchr (haystack, tolower (*needle)); // find occurence of first character (lowercase)
      if ((ptr_upper == NULL) && (ptr_lower == NULL)) break; // if no occurence in either case, then haystack doesn't contain needle
      else if (ptr_upper == NULL) ptr_either = ptr_lower; // no uppercase, check in lowercase
      else if (ptr_lower == NULL) ptr_either = ptr_upper; // no lowercase, check in uppercase
      else if (ptr_lower < ptr_upper) ptr_either = ptr_lower; // both occurences found, take the first one
      else ptr_either = ptr_upper; // both occurences found, take the first one
      if (strnicmp (ptr_either, needle, needle_length) == 0) // now compare needle case insensitively at that position in haystack
         return ((char *) ptr_either); // if we find something, return its position
      haystack = ptr_either + 1; // else advance in haystack
   }
   return (NULL); // haystack doesn't contain needle
}
#endif

#ifndef strrstr
static char *strrstr (const char *haystack, const char *needle)
{
   // windows has no strrstr() implementation, so here is mine.
   int haystack_length, needle_length, index;
   haystack_length = strlen (haystack);
   needle_length = strlen (needle);
   if (needle_length > haystack_length) return (NULL); // consistency check: needle to big to fit in haystack
   for (index = haystack_length - needle_length; index >= 0; index--) // scan the string backwards and compare with needle
      if (strncmp (&haystack[index], needle, needle_length) == 0) return ((char *) &haystack[index]); // return string location when we find it
   return (NULL); // no occurence, haystack doesn't contain needle
}
#endif

#ifndef strristr
static char *strristr (const char *haystack, const char *needle)
{
   // windows has no strristr() implementation, so here is mine.
   char *ptr_haystack;
   int needle_length;
   needle_length = strlen (needle);
   for (ptr_haystack = (char *) haystack + strlen (haystack); ptr_haystack >= haystack; ptr_haystack--) // scan string backwards and compare
      if (strnicmp (ptr_haystack, needle, needle_length) == 0) return (ptr_haystack); // return string location when we find it
   return (NULL); // no occurence, haystack doesn't contain needle
}
#endif

#ifndef sgets
static char *sgets (char *destination_line, int max_length, char *source_buffer)
{
   // copy a line from a given string. Kinda like fgets() when you're reading from a string.
   char *pointer;
   int index;
   if (*source_buffer == 0) { destination_line[0] = 0; return (NULL); } // if EOS return a NULL pointer
   pointer = strstr (source_buffer, "\n"); // get to the first carriage return we can find
   if (pointer == NULL)
   {
      for (index = 0; index < max_length; index++)
      {
         destination_line[index] = source_buffer[index]; // copy the line we found
         if (source_buffer[index] == 0)
            break; // don't copy beyond the end of source
      }
      destination_line[max_length - 1] = 0; // terminate string
      return (source_buffer + strlen (source_buffer)); // and return the new source buffer pointer
   }
   *pointer = 0; // temporarily turn the carriage return to an end of string
   for (index = 0; index < max_length; index++)
   {
      destination_line[index] = source_buffer[index]; // copy the line we found
      if (source_buffer[index] == 0)
         break; // don't copy beyond the end of source
   }
   destination_line[max_length - 1] = 0; // terminate string
   *pointer = '\n'; // put the carriage return back
   return (pointer + 1); // and return next line's source buffer pointer
}
#endif

#endif // C99_H
