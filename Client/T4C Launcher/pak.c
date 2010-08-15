// pak.c

#include "client.h"


// global variables for internal use only
static char string8[MAX_STRING8];
static char string16[MAX_STRING16];
static fragmented_pak_t *fragments; // fragmented paks array
static int fragment_count; // number of elements in the previous array


void PakMachine_Init (void)
{
   // things to do before using the pak machine

   // initialize storage pointer for fragmented paks
   fragments = NULL;
   fragment_count = 0;

   // initialize all protocols
   Proto160_Init ();

   return; // pak machine is ready to use
}


void PakMachine_Shutdown (void)
{
   // things to do with the pak machine on server shutdown

   // shutdown all protocols
   Proto160_Shutdown ();

   // free the fragmented paks array
   SAFE_free (&fragments);
   fragment_count = 0;

   return; // pak machine is no longer usable
}


fragmented_pak_t *FragmentedPak_FindById (int id)
{
   // this function finds the right slot in the array of the fragmented datagrams being currently
   // built corresponding to the given header. If no slot is found, it allocates and returns
   // space for a new one.

   int index;

   // loop through all the fragmented datagrams we know and return the right one if we find it
   for (index = 0; index < fragment_count; index++)
      if (fragments[index].id == id)
         return (&fragments[index]); // we found it

   // pak not found, check if one of the existing ones can be overwritten
   for (index = 0; index < fragment_count; index++)
      if (fragments[index].arrival_date + 30 < current_time)
      {
         fragments[index].arrival_date = 0; // clean up that datagram's storage space
         memset (fragments[index].arrived_parts, 0, sizeof (fragments[index].arrived_parts));
         fragments[index].parts_count = 0;
         fragments[index].last_fragment_size = 0;
         fragments[index].id = 0;
         return (&fragments[index]); // and finally, return its slot
      }

   // no suitable slot could be found, so we must allocate a new one
   fragments = (fragmented_pak_t *) SAFE_realloc (fragments, fragment_count, fragment_count + 1, sizeof (fragmented_pak_t), false);
   fragment_count++; // we know now one fragmented datagram more

   fragments[index].arrival_date = 0; // clean up that datagram's storage space
   memset (fragments[index].arrived_parts, 0, sizeof (fragments[index].arrived_parts));
   fragments[index].parts_count = 0;
   fragments[index].last_fragment_size = 0;
   fragments[index].id = 0;
   return (&fragments[index]); // and return the slot we just allocated
}


bool BuildPakFromUDPDatagrams (datagram_t *datagram, int crypto_version, pak_t *pak)
{
   // this function builds a T4C pak from a network datagram, or a part of it if it's a fragmented pak

   datagram_t *server_datagram;
   bool return_value;
   int index;

   // first off, ensure we know where the datagram comes from
   if (!(pak->flags & PAKFLAG_CLIENT_PAK) && !(pak->flags & PAKFLAG_SERVER_PAK))
   {
      Log_Datagram (LOG_MAIN, datagram, "BuildPakFromUDPDatagrams (): found datagram of unknown origin!\n");
      return (false); // refuse to build paks if we don't know their origin
   }

   // given the crypto version number, use the right protocol
   if (crypto_version == PAKCRYPTO_VERSION_160)
      return_value = Proto160_BuildPakFromUDPDatagrams (datagram, pak); // use the 1.63 protocol
   else
   {
      Log_Datagram (LOG_MAIN, datagram, "BuildPakFromUDPDatagrams(): unsupported crypto version: %d\n", crypto_version);
      return (false); // unsupported crypto version !
   }

   // is it an ack reply ?
   if (pak->flags & PAKFLAG_ACKREPLY)
   {
      // cycle through all our datagrams to see if it's one of ours
      for (index = 0; index < datagram_count; index++)
      {
         server_datagram = &datagrams[index]; // quick access to datagram

         if (!server_datagram->is_pending || !server_datagram->is_reliable || (server_datagram->associated_id != pak->id))
            continue; // skip invalid datagrams and those we aren't concerned about

         // this is the datagram we were waiting an ack reply for
         server_datagram->is_pending = false; // so forget it now
         return_value = false; // also forget the ack reply
         break; // and stop searching
      }
   }

   return (return_value); // finished, tell the network loop whether to wait for another datagram or not
}


void BuildUDPDatagramsFromPak (pak_t *pak, int crypto_version, int socket_from, address_t *address_to)
{
   // this function builds one or several UDP datagrams ready to be sent over the network from a T4C pak

   // first off, ensure we know where the pak comes from
   if (!(pak->flags & PAKFLAG_CLIENT_PAK) && !(pak->flags & PAKFLAG_SERVER_PAK))
   {
      Log_Pak (LOG_MAIN, pak, "BuildUDPDatagramsFromPak (): found pak of unknown origin!\n");
      return; // refuse to send paks if we don't know their origin
   }

   // given the crypto version number, use the right protocol
   if (crypto_version == PAKCRYPTO_VERSION_160)
      Proto160_BuildUDPDatagramsFromPak (pak, socket_from, address_to);
   else
   {
      Log_Pak (LOG_MAIN, pak, "BuildUDPDatagramsFromPak (): unsupported crypto version: %d\n", crypto_version);
      return; // unsupported crypto version !
   }

   return; // datagram list successfully built
}


void BuildAckReplyDatagramsFromPak (pak_t *pak, int crypto_version, int socket_from, address_t *address_to)
{
   // this function builds one or several ack reply datagrams ready to be sent over the network
   // from a dropped T4C pak

   // first off, ensure we know where the pak comes from
   if (!(pak->flags & PAKFLAG_CLIENT_PAK) && !(pak->flags & PAKFLAG_SERVER_PAK))
   {
      Log_Pak (LOG_MAIN, pak, "BuildAckReplyDatagramsFromPak (): found pak of unknown origin!\n");
      return; // refuse to send paks if we don't know their origin
   }

   // given the crypto version number, use the right protocol
   if (crypto_version == PAKCRYPTO_VERSION_160)
      Proto160_BuildAckReplyDatagramsFromPak (pak, socket_from, address_to);
   else
   {
      Log_Pak (LOG_MAIN, pak, "BuildAckReplyDatagramsFromPak (): unsupported crypto version: %d\n", crypto_version);
      return; // unsupported crypto version !
   }

   return; // finished
}


pak_t *Pak_New (int id, unsigned short flags)
{
   // this function allocates some space in memory for a new pak and its data, cleans up the
   // significant header fields of a pak structure and prepares them given the relevant
   // information passed in parameters

   pak_t *pak;

   // first, allocate space for pak
   pak = (pak_t *) SAFE_malloc (1, sizeof (pak_t), false);
   pak->data = NULL;
   pak->pages_count = 0;

   // now set its base parameters
   pak->id = id;
   pak->flags = flags;
   pak->data_size = 0;

   return (pak); // and return a pointer to the pak space we allocated and prepared
}


void Pak_Destroy (pak_t *pak)
{
   // this function frees the memory space that was allocated for a pak and its data and destroys
   // the pointer that pointed to them

   if (pak != NULL)
      SAFE_free (&pak->data); // free the pak data space
   SAFE_free (&pak); // and finally free the pak itself

   return; // simple as that
}


int8 Pak_ReadInt8 (pak_t *pak, unsigned int position)
{
   // this function reads a 8-bit number at position in the pak data

   if ((pak->data == NULL) || (position + sizeof (int8) > pak->data_size))
      return (0); // consistency check

   return (pak->data[position]); // bytes are in reverse order in the pak data
}


int16 Pak_ReadInt16 (pak_t *pak, unsigned int position)
{
   // this function reads a 16-bit number at position in the pak data

   if ((pak->data == NULL) || (position + sizeof (int16) > pak->data_size))
      return (0); // consistency check

   return (((int8) pak->data[position + 0]) << 8
           | ((int8) pak->data[position + 1])); // bytes are in reverse order
}


int32 Pak_ReadInt32 (pak_t *pak, unsigned int position)
{
   // this function reads a 32-bit number at position in the pak data

   if ((pak->data == NULL) || (position + sizeof (int32) > pak->data_size))
      return (0); // consistency check

   return (((int8) pak->data[position + 0]) << 24
           | ((int8) pak->data[position + 1]) << 16
           | ((int8) pak->data[position + 2]) << 8
           | ((int8) pak->data[position + 3])); // bytes are in reverse order
}


int64 Pak_ReadInt64 (pak_t *pak, unsigned int position)
{
   // this function reads a 64-bit number at position in the pak data

   if ((pak->data == NULL) || (position + sizeof (int64) > pak->data_size))
      return (0); // consistency check

   return ((int64) ((int8) pak->data[position + 0]) << 56
           | (int64) ((int8) pak->data[position + 1]) << 48
           | (int64) ((int8) pak->data[position + 2]) << 40
           | (int64) ((int8) pak->data[position + 3]) << 32
           | (int64) ((int8) pak->data[position + 4]) << 24
           | (int64) ((int8) pak->data[position + 5]) << 16
           | (int64) ((int8) pak->data[position + 6]) << 8
           | (int64) ((int8) pak->data[position + 7])); // bytes are in reverse order
}


int Pak_ReadString8 (pak_t *pak, unsigned int position, char *out_string, int max_length)
{
   // this function reads a short string up to max_length characters at position in the pak data
   // and returns the size it claims to have (which can be greater than max_length).

   int length;

   out_string[0] = 0; // start by cleaning the output string

   if ((pak->data == NULL) || (position + sizeof (int8) > pak->data_size))
      return (0); // consistency check: is pak data too short to read even length ?

   length = (int) pak->data[position + 0]; // get string length

   if (position + sizeof (int8) + length > pak->data_size)
      return (0); // consistency check: is pak data too short to read "length" bytes ?

   // clamp string length to max length
   if (length > max_length - 1)
   {
      memcpy (out_string, &pak->data[position + sizeof (int8)], max_length - 1); // get string data
      out_string[max_length - 1] = 0; // terminate string
   }
   else
   {
      memcpy (out_string, &pak->data[position + sizeof (int8)], length); // get string data
      out_string[length] = 0; // terminate string
   }

   return (length); // string has been read from the pak
}


int Pak_ReadString16 (pak_t *pak, unsigned int position, char *out_string, int max_length)
{
   // this function reads a short string up to max_length characters at position in the pak data
   // and returns the size it claims to have (which can be greater than max_length).

   int length;

   out_string[0] = 0; // start by cleaning the output string

   if ((pak->data == NULL) || (position + sizeof (int16) > pak->data_size))
      return (0); // consistency check: is pak data too short to read even length ?

   length = (int) (((int8) pak->data[position + 0]) << 8
                   | ((int8) pak->data[position + 1])); // get string length

   if (position + sizeof (int16) + length > pak->data_size)
      return (0); // consistency check: is pak data too short to read "length" bytes ?

   // clamp string length to max length
   if (length > max_length - 1)
   {
      memcpy (out_string, &pak->data[position + sizeof (int16)], max_length - 1); // get string data
      out_string[max_length - 1] = 0; // terminate string
   }
   else
   {
      memcpy (out_string, &pak->data[position + sizeof (int16)], length); // get string data
      out_string[length] = 0; // terminate string
   }

   return (length); // string has been read from the pak
}


void Pak_WriteInt8 (pak_t *pak, bool update_size, unsigned int position, int8 in_data)
{
   // this function writes a 8-bit number at position in the pak data

   if ((pak == NULL) || (position + sizeof (int8) > USHRT_MAX))
      return; // consistency check

   // if position is farther than pak data size, we need to grow it
   if (update_size || (pak->data_size < position + sizeof (int8)))
   {
      pak->data_size = position + sizeof (int8); // remember new data size

      // if new data size requires us to allocate more pages for pak data, do it
      if (pak->data_size / 1024 + 1 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, pak->data_size / 1024 + 1, 1024, false);
         pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
      }
   }

   pak->data[position] = (unsigned char) in_data;
   return; // bytes are in reverse order in the pak data
}


void Pak_WriteInt16 (pak_t *pak, bool update_size, unsigned int position, int16 in_data)
{
   // this function writes a 16-bit number at position in the pak data

   if ((pak == NULL) || (position + sizeof (int16) > USHRT_MAX))
      return; // consistency check

   // if position is farther than pak data size, we need to grow it
   if (update_size || (pak->data_size < position + sizeof (int16)))
   {
      pak->data_size = position + sizeof (int16); // remember new data size

      // if new data size requires us to allocate more pages for pak data, do it
      if (pak->data_size / 1024 + 1 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, pak->data_size / 1024 + 1, 1024, false);
         pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
      }
   }

   pak->data[position + 0] = (unsigned char) (in_data >> 8);
   pak->data[position + 1] = (unsigned char) (in_data);

   return; // bytes are in reverse order in the pak data
}


void Pak_WriteInt32 (pak_t *pak, bool update_size, unsigned int position, int32 in_data)
{
   // this function writes a 32-bit number at position in the pak data

   if ((pak == NULL) || (position + sizeof (int32) > USHRT_MAX))
      return; // consistency check

   // if position is farther than pak data size, we need to grow it
   if (update_size || (pak->data_size < position + sizeof (int32)))
   {
      pak->data_size = position + sizeof (int32); // remember new data size

      // if new data size requires us to allocate more pages for pak data, do it
      if (pak->data_size / 1024 + 1 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, pak->data_size / 1024 + 1, 1024, false);
         pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
      }
   }

   pak->data[position + 0] = (unsigned char) (in_data >> 24);
   pak->data[position + 1] = (unsigned char) (in_data >> 16);
   pak->data[position + 2] = (unsigned char) (in_data >> 8);
   pak->data[position + 3] = (unsigned char) (in_data);

   return; // bytes are in reverse order in the pak data
}


void Pak_WriteInt64 (pak_t *pak, bool update_size, unsigned int position, int64 in_data)
{
   // this function writes a 64-bit number at position in the pak data

   if ((pak == NULL) || (position + sizeof (int64) > USHRT_MAX))
      return; // consistency check

   // if position is farther than pak data size, we need to grow it
   if (update_size || (pak->data_size < position + sizeof (int64)))
   {
      pak->data_size = position + sizeof (int64); // remember new data size

      // if new data size requires us to allocate more pages for pak data, do it
      if (pak->data_size / 1024 + 1 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, pak->data_size / 1024 + 1, 1024, false);
         pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
      }
   }

   pak->data[position + 0] = (unsigned char) (in_data >> 56);
   pak->data[position + 1] = (unsigned char) (in_data >> 48);
   pak->data[position + 2] = (unsigned char) (in_data >> 40);
   pak->data[position + 3] = (unsigned char) (in_data >> 32);
   pak->data[position + 4] = (unsigned char) (in_data >> 24);
   pak->data[position + 5] = (unsigned char) (in_data >> 16);
   pak->data[position + 6] = (unsigned char) (in_data >> 8);
   pak->data[position + 7] = (unsigned char) (in_data);

   return; // bytes are in reverse order in the pak data
}


void Pak_WriteString8 (pak_t *pak, bool update_size, unsigned int position, char *fmt, ...)
{
   // this function writes a short string at position in the pak data

   va_list argptr;
   int length;

   // concatenate all the arguments of the message
   va_start (argptr, fmt);
   SAFE_vsnprintf (string8, sizeof (string8), fmt, argptr);
   va_end (argptr);

   length = strlen (string8); // get string length

   if ((pak == NULL) || (length >= MAX_STRING8) || (position + sizeof (int8) + length > USHRT_MAX))
      return; // consistency check

   // if string can't fit in the pak data space, we need to grow it
   if (update_size || (pak->data_size < position + sizeof (int8) + length))
   {
      pak->data_size = position + sizeof (int8) + length; // remember new data size

      // if new data size requires us to allocate more pages for pak data, do it
      if (pak->data_size / 1024 + 1 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, pak->data_size / 1024 + 1, 1024, false);
         pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
      }
   }

   pak->data[position + 0] = (unsigned char) length; // write string length
   memcpy (&pak->data[position + sizeof (int8)], string8, length); // write string data

   return; // string has been written in the pak
}


void Pak_WriteString16 (pak_t *pak, bool update_size, unsigned int position, char *fmt, ...)
{
   // this function writes a long string at position in the pak data

   va_list argptr;
   int length;

   // concatenate all the arguments of the message
   va_start (argptr, fmt);
   SAFE_vsnprintf (string16, sizeof (string16), fmt, argptr);
   va_end (argptr);

   length = strlen (string16); // get string length

   if ((pak == NULL) || (length >= MAX_STRING16) || (position + sizeof (int16) + length > USHRT_MAX))
      return; // consistency check

   // if string can't fit in the pak data space, we need to grow it
   if (update_size || (pak->data_size < position + sizeof (int16) + length))
   {
      pak->data_size = position + sizeof (int16) + length; // remember new data size

      // if new data size requires us to allocate more pages for pak data, do it
      if (pak->data_size / 1024 + 1 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, pak->data_size / 1024 + 1, 1024, false);
         pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
      }
   }

   pak->data[position + 0] = (unsigned char) (length >> 8); // write string length
   pak->data[position + 1] = (unsigned char) (length);
   memcpy (&pak->data[position + sizeof (int16)], string16, length); // write string data

   return; // string has been written in the pak
}


void Pak_WriteRawString8 (pak_t *pak, bool update_size, unsigned int position, int8 data_size, char *data)
{
   // this function writes a short raw data string at position in the pak data

   if ((pak == NULL) || (position + sizeof (int8) + data_size > USHRT_MAX))
      return; // consistency check

   // if string can't fit in the pak data space, we need to grow it
   if (update_size || (pak->data_size < position + sizeof (int8) + data_size))
   {
      pak->data_size = position + sizeof (int8) + data_size; // remember new data size

      // if new data size requires us to allocate more pages for pak data, do it
      if (pak->data_size / 1024 + 1 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, pak->data_size / 1024 + 1, 1024, false);
         pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
      }
   }

   pak->data[position + 0] = (unsigned char) data_size; // write string length
   memcpy (&pak->data[position + sizeof (int8)], data, data_size); // write string data

   return; // string has been written in the pak
}


void Pak_WriteRawString16 (pak_t *pak, bool update_size, unsigned int position, int16 data_size, char *data)
{
   // this function writes a long raw data string at position in the pak data

   if ((pak == NULL) || (position + sizeof (int16) + data_size > USHRT_MAX))
      return; // consistency check

   // if string can't fit in the pak data space, we need to grow it
   if (update_size || (pak->data_size < position + sizeof (int16) + data_size))
   {
      pak->data_size = position + sizeof (int16) + data_size; // remember new data size

      // if new data size requires us to allocate more pages for pak data, do it
      if (pak->data_size / 1024 + 1 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, pak->data_size / 1024 + 1, 1024, false);
         pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
      }
   }

   pak->data[position + 0] = (unsigned char) (data_size >> 8); // write string length
   pak->data[position + 1] = (unsigned char) (data_size);
   memcpy (&pak->data[position + sizeof (int16)], data, data_size); // write string data

   return; // string has been written in the pak
}


void Pak_AddInt8 (pak_t *pak, int8 in_data)
{
   // this function appends a 8-bit number to the pak data

   if ((pak == NULL) || (pak->data_size + sizeof (int8) > USHRT_MAX))
      return; // consistency check

   // if new data size requires us to allocate more pages for pak data, do it
   if (((int) pak->data_size + (int) sizeof (int8)) / 1024 + 1 > pak->pages_count)
   {
      pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, (pak->data_size + sizeof (int8)) / 1024 + 1, 1024, false);
      pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
   }

   pak->data[pak->data_size] = (unsigned char) in_data;
   pak->data_size = pak->data_size + sizeof (int8); // remember new data size

   return; // bytes are in reverse order in the pak data
}


void Pak_AddInt16 (pak_t *pak, int16 in_data)
{
   // this function appends a 16-bit number to the pak data

   if ((pak == NULL) || (pak->data_size + sizeof (int16) > USHRT_MAX))
      return; // consistency check

   // if new data size requires us to allocate more pages for pak data, do it
   if (((int) pak->data_size + (int) sizeof (int16)) / 1024 + 1 > pak->pages_count)
   {
      pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, (pak->data_size + sizeof (int16)) / 1024 + 1, 1024, false);
      pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
   }

   pak->data[pak->data_size + 0] = (unsigned char) (in_data >> 8);
   pak->data[pak->data_size + 1] = (unsigned char) (in_data);
   pak->data_size = pak->data_size + sizeof (int16); // remember new data size

   return; // bytes are in reverse order in the pak data
}


void Pak_AddInt32 (pak_t *pak, int32 in_data)
{
   // this function appends a 32-bit number to the pak data

   if ((pak == NULL) || (pak->data_size + sizeof (int32) > USHRT_MAX))
      return; // consistency check

   // if new data size requires us to allocate more pages for pak data, do it
   if (((int) pak->data_size + (int) sizeof (int32)) / 1024 + 1 > pak->pages_count)
   {
      pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, (pak->data_size + sizeof (int32)) / 1024 + 1, 1024, false);
      pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
   }

   pak->data[pak->data_size + 0] = (unsigned char) (in_data >> 24);
   pak->data[pak->data_size + 1] = (unsigned char) (in_data >> 16);
   pak->data[pak->data_size + 2] = (unsigned char) (in_data >> 8);
   pak->data[pak->data_size + 3] = (unsigned char) (in_data);
   pak->data_size = pak->data_size + sizeof (int32); // remember new data size

   return; // bytes are in reverse order in the pak data
}


void Pak_AddInt64 (pak_t *pak, int64 in_data)
{
   // this function appends a 64-bit number to the pak data

   if ((pak == NULL) || (pak->data_size + sizeof (int64) > USHRT_MAX))
      return; // consistency check

   // if new data size requires us to allocate more pages for pak data, do it
   if (((int) pak->data_size + (int) sizeof (int64)) / 1024 + 1 > pak->pages_count)
   {
      pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, (pak->data_size + sizeof (int64)) / 1024 + 1, 1024, false);
      pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
   }

   pak->data[pak->data_size + 0] = (unsigned char) (in_data >> 56);
   pak->data[pak->data_size + 1] = (unsigned char) (in_data >> 48);
   pak->data[pak->data_size + 2] = (unsigned char) (in_data >> 40);
   pak->data[pak->data_size + 3] = (unsigned char) (in_data >> 32);
   pak->data[pak->data_size + 4] = (unsigned char) (in_data >> 24);
   pak->data[pak->data_size + 5] = (unsigned char) (in_data >> 16);
   pak->data[pak->data_size + 6] = (unsigned char) (in_data >> 8);
   pak->data[pak->data_size + 7] = (unsigned char) (in_data);
   pak->data_size = pak->data_size + sizeof (int64); // remember new data size

   return; // bytes are in reverse order in the pak data
}


void Pak_AddString8 (pak_t *pak, char *fmt, ...)
{
   // this function appends a short string to the pak data

   va_list argptr;
   int length;

   // concatenate all the arguments of the message
   va_start (argptr, fmt);
   SAFE_vsnprintf (string8, sizeof (string8), fmt, argptr);
   va_end (argptr);

   length = strlen (string8); // get string length

   if ((pak == NULL) || (length >= MAX_STRING8) || (pak->data_size + sizeof (int8) + length > USHRT_MAX))
      return; // consistency check

   // if new data size requires us to allocate more pages for pak data, do it
   if (((int) pak->data_size + (int) sizeof (int8) + length) / 1024 + 1 > pak->pages_count)
   {
      pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, (pak->data_size + sizeof (int8) + length) / 1024 + 1, 1024, false);
      pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
   }

   pak->data[pak->data_size + 0] = (unsigned char) length; // write string length
   memcpy (&pak->data[pak->data_size + sizeof (int8)], string8, length); // write string data
   pak->data_size = pak->data_size + sizeof (int8) + length; // remember new data size

   return; // string has been written in the pak
}


void Pak_AddString16 (pak_t *pak, char *fmt, ...)
{
   // this function appends a long string to the pak data

   va_list argptr;
   int length;

   // concatenate all the arguments of the message
   va_start (argptr, fmt);
   SAFE_vsnprintf (string16, sizeof (string16), fmt, argptr);
   va_end (argptr);

   length = strlen (string16); // get string length

   if ((pak == NULL) || (length >= MAX_STRING16) || (pak->data_size + sizeof (int16) + length > USHRT_MAX))
      return; // consistency check

   // if new data size requires us to allocate more pages for pak data, do it
   if (((int) pak->data_size + (int) sizeof (int16) + length) / 1024 + 1 > pak->pages_count)
   {
      pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, (pak->data_size + sizeof (int16) + length) / 1024 + 1, 1024, false);
      pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
   }

   pak->data[pak->data_size + 0] = (unsigned char) (length >> 8); // write string length
   pak->data[pak->data_size + 1] = (unsigned char) (length);
   memcpy (&pak->data[pak->data_size + sizeof (int16)], string16, length); // write string data
   pak->data_size = pak->data_size + sizeof (int16) + length; // remember new data size

   return; // string has been written in the pak
}


void Pak_AddRawString8 (pak_t *pak, int8 data_size, char *data)
{
   // this function appends a short string to the pak data

   if ((pak == NULL) || (pak->data_size + sizeof (int8) + data_size > USHRT_MAX))
      return; // consistency check

   // if new data size requires us to allocate more pages for pak data, do it
   if (((int) pak->data_size + (int) sizeof (int8) + data_size) / 1024 + 1 > pak->pages_count)
   {
      pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, (pak->data_size + sizeof (int8) + data_size) / 1024 + 1, 1024, false);
      pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
   }

   pak->data[pak->data_size + 0] = (unsigned char) data_size; // write string length
   memcpy (&pak->data[pak->data_size + sizeof (int8)], data, data_size); // write string data
   pak->data_size = pak->data_size + sizeof (int8) + data_size; // remember new data size

   return; // string has been written in the pak
}


void Pak_AddRawString16 (pak_t *pak, int16 data_size, char *data)
{
   // this function appends a long string to the pak data

   if ((pak == NULL) || (pak->data_size + sizeof (int16) + data_size > USHRT_MAX))
      return; // consistency check

   // if new data size requires us to allocate more pages for pak data, do it
   if (((int) pak->data_size + (int) sizeof (int16) + data_size) / 1024 + 1 > pak->pages_count)
   {
      pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, (pak->data_size + sizeof (int16) + data_size) / 1024 + 1, 1024, false);
      pak->pages_count = pak->data_size / 1024 + 1; // new number of pages
   }

   pak->data[pak->data_size + 0] = (unsigned char) (data_size >> 8); // write string length
   pak->data[pak->data_size + 1] = (unsigned char) (data_size);
   memcpy (&pak->data[pak->data_size + sizeof (int16)], data, data_size); // write string data
   pak->data_size = pak->data_size + sizeof (int16) + data_size; // remember new data size

   return; // string has been written in the pak
}
