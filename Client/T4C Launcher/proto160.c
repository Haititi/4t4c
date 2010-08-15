#include "client.h"


/*
F9 2E FF 9F F9 FF 98 F9 FF 99 BA D0 F7 A8 BA D1 66

89 B8 FF 9F F9 FF 98 F9 FF 99 17 25 C4


A9 5E FF 9F F9 FF 98 F9 FF 99 D7 A8 D5 F5 1D FE D6

B1 86 FF 9F F9 FF 98 F9 FF 99 1F D4 17


F9 78 FF 9F F9 FF 98 F9 FF 99 56 FC 7F 81 F5 A4 E5

51 F2 FF 9F F9 FF 98 F9 FF 99 9F F0 6F


59 7E FF 9F F9 FF 98 F9 FF 99 41 95 BA EC 37 EB CC

D9 0E FF 9F F9 FF 98 F9 FF 99 7D 5B 82


99 6D FF 9F F9 FF 98 F9 FF 99 C2 D4 43 F3 09 C6 A0

49 E4 FF 9F F9 FF 98 F9 FF 99 8F 80 05


19 66 FF 9F F9 FF 98 F9 FF 99 68 A5 CE FB 68 F4 90

41 D9 FF 9F F9 FF 98 F9 FF 99 E9 6A D4


B9 59 FF 9F F9 FF 98 F9 FF 99 DE E0 DE E0 22 D8 B9

69 C6 FF 9F F9 FF 98 F9 FF 99 7C 89 0D


F1 55 FF 9F F9 FF 98 F9 FF 99 22 D5 86 B4 72 98 C0

A9 3E FF 9F F9 FF 98 F9 FF 99 8B F1 DE


31 46 FF 9F F9 FF 98 F9 FF 99 59 E3 A6 81 3F B7 71

D1 02 FF 9F F9 FF 98 F9 FF 99 26 FC 4C
*/


// PAK STRUCTURE:
//
// [ 2bytes ][ 4 bytes ][?? bytes][ 1 byte ]
// [id+flags][signature][pak data][checksum]
// or
// [ 2bytes ][ 4 bytes ][?? bytes][ 1 byte ][ 4 bytes ]
// [id+flags][signature][pak data][checksum][1.50 seed]
//
// All the datagrams are XORed BUT the optional 1.50 seed, with 0x99 for the first 512 bytes,
// and 0x2f for the 512 last.
// If the flags specify it, signature and pak data are encrypted with the 1.50 algorithm and a
// seed is appended uncrypted at the end of the datagram.
// ID takes the first 13 bits and flags the last 3 of the header's two bytes read as a raw short.
// Signatures for normal paks are 0x00000000, signature for "evil" paks are 0x66600666.

// Pak IDs must increment correctly. See below.


// PAK DECRYPTION:
//         9c 2d 65 5e 79 e6 88 73 1a 29 a0 d0 4d
// 1. xor^(99 99) to get the header
//         05 b4
// 2. get header & flags (swap bytes) and signature
//         b405 = 1011010000000 101 = ID 5760 (single pak), unknown + ack requested
//   signature ((65 5e 79 e6 != 99 99 99 99)
//           && (65 5e 79 e6 != ff 9f f9 ff)) => 1.50 encryption 
// 3. determine structure
//         [HEA] [SIGNATURE] [DAT] [] [SEED-1.50]
// 4. verify 1 byte checksum = 0 on all the raw data BUT the 1.50 seed
//         9c+2d+65+5e+79+e6+88+73+1a = 0
// 5. xor rest ^(99 99 99 99 99 99) to get the signature and the data
//               fc c7 e0 7f 11 ea
// 6. decypher using seed-1.50 -----> 29 a0 d0 4d
//               00 00 00 00 00 42
// 7. get signature & data
//               0x00000000 / 00 42 = normal pak / PAK_CLIENT_MessageOfTheDay

// EVIL PAK
// ----------------------------------------------------------
// Request: client to server
//         F9 2E FF 9F F9 FF 98 F9 FF 99 BA D0 F7 A8 BA D1 66
// 1.      60 b7
// 2.      b7 60 = 1011011101100 000
// 3.      [HEA] [SIGNATURE] [           DATA            ] []
// 4.      f9+2e+ff+9f+f9+ff+98+f9+ff+99+ba+d0+f7+a8+ba+d1+66 = 0
// 5.            66 06 60 66 01 60 66 00 23 49 6e 31 23 48
// 6.      --
// 7.            [x66600666] [x00666001] 23 49 6e 31 23 48
// ----------------------------------------------------------
// Reply: server to client
//         89 B8 FF 9F F9 FF 98 F9 FF 99 17 25 C4
// 1.      10 21
// 2.      21 10 = 0010000100010 000
// 3.      [HEA] [SIGNATURE] [     DATA      ] []
// 4.      89+b8+ff+9f+f9+ff+98+f9+ff+99+17+25+c4 = 0
// 5.            66 06 60 66 01 60 66 00 8e bc
// 6.      --
// 7.            [x66600666] [x00666001] 8e bc
// 0xBC8E is the CRC16-CCITT (with a CRC start value of 0x0000) of "\x23\x49\x6e\x31\x23\x48"

// PAK FRAGMENTATION:
// ----------------------------[HDR]--[DATA+CHECKSUM]-(hdr XOR + byteswap)----------------
// 4 datagrams fragmented pak: 98 98  99 99 99 99 ... (00000001 00000 001) 00 00 00 00 ...
//                             90 98  fd eb f8 b9 ... (00000001 00001 001) 64 72 61 20 ...
//                             88 98  fb f0 fc eb ... (00000001 00010 001) 62 69 65 72 ...
//                             82 98  eb f8 fe f6 ... (00000001 00011 011) 72 61 67 6f ...
//                                                    (byteswap only)
// corresponding acks:         00 01                  (00000001 00000 000)
//                             08 01                  (00000001 00001 000)
//                             10 01                  (00000001 00010 000)
//                             18 01                  (00000001 00011 000)
// ----------------------------[HDR]--[DATA+CHECKSUM]-(hdr XOR + byteswap)----------------
// 2 datagrams fragmented pak: 98 9b  99 99 99 99 ... (00000010 00000 001) 00 00 00 00 ...
//                             92 9b  99 92 c1 9d ... (00000010 00001 011) 00 0b 58 04 ...
//                                                    (byteswap only)
// corresponding acks:         00 02                  (00000010 00000 000)
//                             08 02                  (00000010 00001 000)
// ----------------------------[HDR]--[DATA+CHECKSUM]-(hdr XOR + byteswap)----------------
// 2 datagrams fragmented pak: 98 9a  99 99 99 99 ... (00000011 00000 001) 00 00 00 00 ...
//                             92 9a  fd d7 00 00 ... (00000011 00001 011) 64 4e 99 99 ...
//                                                    (byteswap only)
// corresponding acks:         00 03                  (00000011 00000 000)
//                             08 03                  (00000011 00001 000)
// ----------------------------[HDR]--[DATA+CHECKSUM]-(hdr XOR + byteswap)----------------
// 2 datagrams fragmented pak: 98 9d  99 99 99 99 ... (00000100 00000 001) 00 00 00 00 ...
//                             92 9d  73 99 99 99 ... (00000100 00001 011) ea 00 00 00 ...
//                                                    (byteswap only)
// corresponding acks:         00 04                  (00000100 00000 000)
//                             08 04                  (00000100 00001 000)
// ----------------------------[HDR]--[DATA+CHECKSUM]-(hdr XOR + byteswap)----------------
// 2 datagrams fragmented pak: 98 9c  99 99 99 99 ... (00000101 00000 001) 00 00 00 00 ...
//                             92 9c  73 99 99 99 ... (00000101 00001 011) ea 00 00 00 ...
//                                                    (byteswap only)
// corresponding acks:         00 05                  (00000101 00000 000)
//                             08 05                  (00000101 00001 000)
// ----------------------------[HDR]--[DATA+CHECKSUM]-(hdr XOR + byteswap)----------------
// 6 datagrams fragmented pak: 99 98  99 99 99 99 ... (00000001 00000 000) 00 00 00 00 ...
//                             91 98  f6 ea ed eb ... (00000001 00001 000) 6f 73 74 72 ...
//                             89 98  ed b9 70 ed ... (00000001 00010 000) 74 20 e9 74 ...
//                             81 98  d5 f0 fa f1 ... (00000001 00011 000) 4c 69 63 68 ...
//                             b9 98  ec ff ff f0 ... (00000001 00100 000) 75 66 66 69 ...
//                             b3 98  ee f8 f5 f5 ... (00000001 00101 010) 77 61 6c 6c ...
// ----------------------------[HDR]--[DATA+CHECKSUM]-(hdr XOR + byteswap)----------------

// PAK SEQUENCING (NORMAL PAKS):
// normal pak:   9c 2d   1011010000000 101
// normal pak:   94 2d   1011010000001 101
// normal pak:   88 2d   1011010000010 001
// normal pak:   81 2d   1011010000011 000
// normal pak:   b9 2d   1011010000100 000
// normal pak:   b5 2d   1011010000101 100
// normal pak:   ad 2d   1011010000110 100
// normal pak:   a5 2d   1011010000111 100
// normal pak:   d9 2d   1011010001000 000
// normal pak:   d5 2d   1011010001001 100
// normal pak:   c9 2d   1011010001010 000
// ...
// normal pak:   64 2d   1011010011111 101
// normal pak:   98 2c   1011010100000 001
// ...
// normal pak:   65 2c   1011010111111 100
// normal pak:   9d 2f   1011011000000 100
// ...
// normal pak:   64 2e   1011011111111 101
// normal pak:   98 21   1011100000000 001
// normal pak:   94 21   1011100000001 101
// ...
// normal pak:   65 22   1011101111111 100
// normal pak:   9d 25   1011110000000 100
// ...
// normal pak:   60 26   1011111111111 001
// normal pak:   9c 59   1100000000000 101
// ...
// normal pak:   6d 66   1111111111110 100
// normal pak:   99 2d   1011010000000 000


// T4C 1.60 pak header types (bitmapped)
#define PAKHEADER_160_TYPE_FRAGMENTCRYPTO150 (unsigned char) (1 << 2)
#define PAKHEADER_160_TYPE_FRAGMENTTAIL (unsigned char) (1 << 1)
#define PAKHEADER_160_TYPE_ACKREQUEST (unsigned char) (1 << 0)


// evil paks signature
#define PAKSIGNATURE_160_EVILPAK 0x66600666
#define PAKSIGNATURE_160_EVILPAK_NEXT 0x00666001


// acks correspondence table
typedef struct ack_160_s
{
   int original_id; // ID of the pak in the uncrypted base 1.25 protocol
   short final_160_id; // ID of the pak in the 1.60 protocol
   float arrival_date; // date at which this ack request was asked
} ack_160_t;


// cryptographic arrays
typedef struct cryptotables_160_s
{
   unsigned long xor[USHRT_MAX]; // table holding the 4 XOR values for each byte of the datagram
   unsigned long algo[USHRT_MAX]; // table holding specific encryption algo data for each byte
   unsigned long offsets[USHRT_MAX]; // table holding offset algo data for each byte
} cryptotables_160_t;


// function prototypes for internal use only
static signed short Pak_ComputeChecksum_160 (pak_t *pak);
static void Pak_SwitchEncryptionOff_160 (pak_t *pak);
static bool Datagram_DeXOR_160 (datagram_t *datagram, int length);
static void Datagram_EnXOR_160 (datagram_t *datagram, int length);
static ack_160_t *Ack160_FindByOriginalId (int original_id);
static ack_160_t *Ack160_FindBy160Id (unsigned short id_160);
static unsigned short CRC16_160 (unsigned char *data, int length);


// global variables for internal use only
static cryptotables_160_t cryptotables_160;
static datagram_t datagram_160;
static char textbuffer_160[MAX_STRING16];
static unsigned char DATAGRAM_DECRYPT_160_MASK[2] = {0x99, 0x2F};
static unsigned short normal_pak_id_counter;
static unsigned short fragmented_pak_id_counter;
static ack_160_t *acks_160; // mallocated
static int ack_count_160;


void Proto160_Init (void)
{
   // this function is called on network layer initialization, to prepare anything that's needed
   // in this version of the T4C protocol, such as allocating arrays, etc.

   acks_160 = NULL; // initialize the acks table array
   ack_count_160 = 0;

   normal_pak_id_counter = 5760; // initialize the normal pak ID counter
   fragmented_pak_id_counter = 1; // initialize the fragmented paks ID counter

   return; // finished
}


void Proto160_Shutdown (void)
{
   // this function is called on network layer shutdown, to clean up everything that was needed
   // in this version of the T4C protocol, such as freeing memory space, resetting variables, etc.

   normal_pak_id_counter = 0; // reset the normal pak ID counter
   fragmented_pak_id_counter = 0; // reset the fragmented paks ID counter

   SAFE_free (&acks_160); // free the memory space we allocated for the acks table array
   ack_count_160 = 0;

   return; // finished
}


bool Proto160_BuildPakFromUDPDatagrams (datagram_t *datagram, pak_t *pak)
{
   // this function builds a T4C pak from a network datagram, or a part of it if it's a fragmented pak

   fragmented_pak_t *fragmented;
   ack_160_t *ack_160;
   unsigned short header_word;
   unsigned long signature;
   unsigned char flags;
   unsigned long evil_mark;
   unsigned char challenge[6];
   int fragment_number;
   int index;

   // is the datagram too short to be a T4C pak ? (even a header doesn't fit)
   if (datagram->size < 2)
   {
      Log_Datagram (LOG_MAIN, datagram, "Proto160_BuildPakFromUDPDatagrams(): dropping invalid datagram (no header).\n");
      return (false); // this ain't a valid datagram
   }

   // read the datagram's header
   memcpy (&header_word, datagram->bytes, 2);

   // is it an ack reply ?
   if (datagram->size == 2)
   {
      // is it a bogus ack reply ? (i.e, it does not contain ONLY the datagram ID)
      if ((header_word & 0x0007) != 0)
      {
         Log_Datagram (LOG_MAIN, datagram, "Proto160_BuildPakFromUDPDatagrams(): dropping datagram (malformed ack).\n");
         return (false); // this ain't a valid packet
      }

      // find the ack it is in our correspondence table
      ack_160 = Ack160_FindBy160Id ((unsigned short) ((header_word & 0xFFF8) >> 3));

      // get just the pak ID (there's no seed, checksum or data)
      pak->id = ack_160->original_id;
      pak->flags |= PAKFLAG_ACKREPLY;
      pak->data_size = 0; // this pak contains no data, reset size to zero

      // now that we've identified this ack, we no longer need to remember it, so reset its slot
      memset (ack_160, 0, sizeof (ack_160_t));
      return (true); // pak successfully built
   }

   // now that we have the datagram and its header, ensure we have all the data

   header_word ^= 0x9999; // unXOR the header
   flags = (unsigned char) (header_word & 0x0007); // get datagram flags

   // is it a fragmented pak ? (fragmented paks are longer than 1023 and sent in several shots)
   if ((datagram->size == 1024) || (flags & PAKHEADER_160_TYPE_FRAGMENTTAIL))
   {
      // deXOR the datagram and control its checksum
      if (!Datagram_DeXOR_160 (datagram, ((flags & PAKHEADER_160_TYPE_FRAGMENTCRYPTO150) & (flags & PAKHEADER_160_TYPE_FRAGMENTTAIL)) ? datagram->size - 4 : datagram->size))
      {
         Log_Datagram (LOG_MAIN, datagram, "Proto160_BuildPakFromUDPDatagrams(): dropping invalid datagram (bad checksum).\n");
         return (false); // this ain't a valid datagram
      }

      // if it's a fragmented pak, split the ID between the master ID and the fragment number
      fragment_number = (unsigned long) ((header_word & 0x00F8) >> 3);
      fragmented = FragmentedPak_FindById ((header_word & 0xFF00) >> 8);

      fragmented->id = (header_word & 0xFF00) >> 8; // ensure master ID is saved
      fragmented->arrival_date = current_time; // save arrival date

      // copy the pak data at the right position in the linearized buffer
      memcpy (fragmented->bytes + fragment_number * (1024 - 1 - 2), datagram->bytes + 2, datagram->size - 2 - 1);
      fragmented->arrived_parts[fragment_number] = true; // remember this part is arrived

      // was it the end bit ?
      if (flags & PAKHEADER_160_TYPE_FRAGMENTTAIL)
         fragmented->parts_count = fragment_number + 1; // if so, remember datagram's part count
      else if (fragmented->parts_count == 0)
         fragmented->parts_count = 32; // else if unspecified yet, assume 32 parts max

      // if it's the last fragment, remember its size
      if (fragment_number + 1 == fragmented->parts_count)
         fragmented->last_fragment_size = datagram->size - 2;

      // is the data NOT completely arrived ? cycle through all parts...
      for (index = 0; index < fragmented->parts_count; index++)
         if (!fragmented->arrived_parts[index])
            return (false); // if at least one part isn't arrived yet, wait for the next one

      // pak is fully arrived, copy it out

      // build the pak ID and miscellaneous flags
      pak->id = fragmented->id;
      if (flags & PAKHEADER_160_TYPE_ACKREQUEST)
      {
         pak->flags |= PAKFLAG_ACKREQUESTED; // remember this pak is an ack request

         // and build (or update) an entry for it in the correspondence table
         ack_160 = Ack160_FindBy160Id ((unsigned short) ((header_word & 0xFFF8) >> 3));
         ack_160->arrival_date = current_time; // remember arrival date
         ack_160->original_id = pak->id + fragment_number;
         ack_160->final_160_id = (header_word & 0xFFF8) >> 3; // ID 8 bit + fragment number 5 bit
      }

      // figure out full data size
      if (flags & PAKHEADER_160_TYPE_FRAGMENTCRYPTO150)
         pak->data_size = (fragmented->parts_count - 1) * (1024 - 1 - 2) + fragmented->last_fragment_size - 4;
      else
         pak->data_size = (fragmented->parts_count - 1) * (1024 - 1 - 2) + fragmented->last_fragment_size;

      // reallocate enough pages to hold pak data if needed
      if (1 + pak->data_size / 1024 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, 1 + pak->data_size / 1024, 1024, false);
         pak->pages_count = 1 + pak->data_size / 1024; // new number of pages
      }

      // copy pak data
      memcpy (pak->data, fragmented->bytes, pak->data_size);

      // read the pak signature once the pak is assembled (this one is already unXORed)
      memcpy (&signature, pak->data, sizeof (unsigned long));

      // clean up the fragmented pak information
      fragmented->arrival_date = 0;
      memset (fragmented->arrived_parts, 0, sizeof (fragmented->arrived_parts));
      fragmented->parts_count = 0;
      fragmented->last_fragment_size = 0;
      fragmented->id = 0;
   }

   // else it's NOT a fragmented pak
   else
   {
      // read the pak signature and unXOR it
      memcpy (&signature, datagram->bytes + 2, sizeof (unsigned long));
      signature ^= 0x99999999;

      // unXOR the datagram and control its checksum
      if (!Datagram_DeXOR_160 (datagram, ((signature == 0) || (signature == PAKSIGNATURE_160_EVILPAK) ? datagram->size : datagram->size - 4)))
      {
         Log_Datagram (LOG_MAIN, datagram, "Proto160_BuildPakFromUDPDatagrams(): dropping invalid datagram (bad checksum).\n");
         return (false); // this ain't a valid datagram
      }

      // build the pak ID and miscellaneous flags
      pak->id = GetTickCount () % RAND_MAX;
      if (flags & PAKHEADER_160_TYPE_ACKREQUEST)
      {
         pak->flags |= PAKFLAG_ACKREQUESTED;

         // and build (or update) an entry for it in the correspondence table
         ack_160 = Ack160_FindBy160Id ((unsigned short) ((header_word & 0xFFF8) >> 3));
         ack_160->arrival_date = current_time; // remember arrival date
         ack_160->original_id = pak->id;
         ack_160->final_160_id = (header_word & 0xFFF8) >> 3; // ID 8 bit + fragment number 5 bit
      }

      // figure out pak data size
      pak->data_size = datagram->size - 2;

      // reallocate enough pages to hold pak data if needed
      if (1 + pak->data_size / 1024 > pak->pages_count)
      {
         pak->data = (char *) SAFE_realloc (pak->data, pak->pages_count, 1 + pak->data_size / 1024, 1024, false);
         pak->pages_count = 1 + pak->data_size / 1024; // new number of pages
      }

      // copy pak data
      memcpy (pak->data, datagram->bytes + 2, pak->data_size);
   }

   // remove Vircom encryption from the pak if needed
   if (pak->flags & PAKFLAG_CRYPTO_VIRCOM)
   {
      if ((signature != 0) && (signature != PAKSIGNATURE_160_EVILPAK))
         Pak_SwitchEncryptionOff_160 (pak); // remove 1.50 encryption only if necessary
      pak->flags &= ~PAKFLAG_CRYPTO_VIRCOM; // this pak no longer wears Vircom encryption
   }

   // is it an evil pak ?
   if (signature == PAKSIGNATURE_160_EVILPAK)
   {
      // is pak NOT long enough to be an evil pak ?
      if (pak->data_size < 14)
      {
         Log_Datagram (LOG_MAIN, datagram, "Proto160_BuildPakFromUDPDatagrams(): dropping invalid datagram (evil signature but not enough data).\n");
         return (false); // this ain't a valid datagram
      }

      // read the second long and see if it's the evil mark
      memcpy (&evil_mark, pak->data + 4, sizeof (unsigned long));
      if (evil_mark != PAKSIGNATURE_160_EVILPAK_NEXT)
      {
         Log_Datagram (LOG_MAIN, datagram, "Proto160_BuildPakFromUDPDatagrams(): dropping invalid datagram (evil signature but no evil mark).\n");
         return (false); // this ain't a valid datagram
      }

      // now we are certain this is an evil pak. Read the challenge to compute the CRC16 onto
      memcpy (challenge, pak->data + 4 + 4, 6);

      // okay, now transform this evil pak into one that can be understood by the pak engine
      Pak_WriteInt16 (pak, true, 4, (unsigned short) (pak->flags & PAKFLAG_CLIENT_PAK ? PAK_CLIENT_160Challenge : PAK_SERVER_160Response));
      Pak_AddInt16 (pak, CRC16_160 (challenge, 3)); // compute CRC16 for that challenge
      pak->data_size++; // increment size to take in account the byte checksum
   }

   // shift the pak data four bytes backwards to discard the signature
   memmove (pak->data, pak->data + 4, pak->data_size - 4);
   pak->data_size -= 4;

   pak->data_size--; // chop off the byte checksum

   return (true); // pak successfully built
}


void Proto160_BuildUDPDatagramsFromPak (pak_t *pak, int socket_from, address_t *address_to)
{
   // this function builds one or several UDP datagrams ready to be sent over the network from a T4C pak

   ack_160_t *ack_160;
   unsigned short offset;
   unsigned short header_word;
   unsigned short fragment_number;
   unsigned short id_160;

   ack_160 = NULL; // reset ack pointer first

   // now, either the datagram is smaller than 1024 bytes and we send it at once, or it's longer
   // and we have to send it in fragments. Or it can be an ack reply, as well.

   // copy source socket and destination address
   datagram_160.our_socket = socket_from;
   memcpy (&datagram_160.peer_address, address_to, sizeof (address_t));

   // is it an ack reply ?
   if (pak->flags & PAKFLAG_ACKREPLY)
   {
      ack_160 = Ack160_FindByOriginalId (pak->id); // first find the original ack request ID

      // report the pak ID in the ack reply
      header_word = ack_160->final_160_id << 3;
      memcpy (datagram_160.bytes, &header_word, 2);
      datagram_160.size = 2;

      // add this datagram to the list of datagrams to send
      Network_PrepareDatagram (&datagram_160, false, 0);

      // now that we've identified this ack, we no longer need to remember it, so reset its slot
      memset (ack_160, 0, sizeof (ack_160_t));
   }

   // it's not ; then, does the datagram need to be fragmented ?
   else if (2 + 4 + pak->data_size + 1 > 1024)
   {
      // build the pak header
      fragment_number = 0; // this pak is the first fragment

      // compute pak ID from master ID and fragment number
      id_160 = (fragmented_pak_id_counter << 5) | fragment_number;

      header_word = id_160 << 3;
      if (pak->flags & PAKFLAG_ACKREQUESTED)
      {
         header_word |= PAKHEADER_160_TYPE_ACKREQUEST;

         ack_160 = Ack160_FindByOriginalId (id_160); // first find the original ack request ID
         ack_160->arrival_date = current_time; // remember arrival date
         ack_160->original_id = pak->id;
         ack_160->final_160_id = id_160;
      }

      // build the master UDP datagram: pak header, signature, and data
      memcpy (datagram_160.bytes, &header_word, 2);
      memset (datagram_160.bytes + 2, 0, 4); // void signature for master datagram
      memcpy (datagram_160.bytes + 2 + 4, pak->data, 1024 - 2 - 4 - 1);
      datagram_160.size = 1024;

      // put back the 1.60 XOR crypto on the datagram and add its byte checksum
      Datagram_EnXOR_160 (&datagram_160, datagram_160.size - 1);

      // add this datagram to the list of datagrams to send
      if ((pak->flags & PAKFLAG_NEW_PAK) && (pak->flags & PAKFLAG_ACKREQUESTED) && (ack_160 != NULL))
         Network_PrepareDatagram (&datagram_160, true, ack_160->original_id);
      else
         Network_PrepareDatagram (&datagram_160, false, 0);

      fragment_number = 1; // one fragment has been built (the master fragment)

      // now, for each of the sub-fragments...
      for (offset = 1024 - 2 - 4 - 1; offset < pak->data_size; offset += 1024 - 1 - 2)
      {
         // build the UDP datagram: fragmented pak header and crypted data immediately after

         // compute pak ID from master ID and fragment number
         id_160 = (fragmented_pak_id_counter << 5) | fragment_number;

         header_word = id_160 << 3;
         if (pak->flags & PAKFLAG_ACKREQUESTED)
         {
            header_word |= PAKHEADER_160_TYPE_ACKREQUEST;

            ack_160 = Ack160_FindByOriginalId (id_160); // first find the original ack request ID
            ack_160->arrival_date = current_time; // remember arrival date
            ack_160->original_id = pak->id + fragment_number;
            ack_160->final_160_id = id_160;
         }

         if (offset + (1024 - 1 - 2) < pak->data_size)
         {
            memcpy (datagram_160.bytes, &header_word, 2);
            memcpy (datagram_160.bytes + 2, pak->data + offset, 1024 - 1 - 2);
            datagram_160.size = 1024;
         }
         else
         {
            header_word |= PAKHEADER_160_TYPE_FRAGMENTTAIL;
            memcpy (datagram_160.bytes, &header_word, 2);
            memcpy (datagram_160.bytes + 2, pak->data + offset, pak->data_size - offset);
            datagram_160.size = pak->data_size - offset + 2 + 1;
         }

         // put back the 1.60 XOR crypto on the datagram and add its byte checksum
         Datagram_EnXOR_160 (&datagram_160, datagram_160.size - 1);

         // add this datagram to the list of datagrams to send
         if ((pak->flags & PAKFLAG_NEW_PAK) && (pak->flags & PAKFLAG_ACKREQUESTED) && (ack_160 != NULL))
            Network_PrepareDatagram (&datagram_160, true, ack_160->original_id);
         else
            Network_PrepareDatagram (&datagram_160, false, 0);

         fragment_number++; // we have built one fragment more
      }

      fragmented_pak_id_counter++; // increment the pak ID (one more has been sent)
      if (fragmented_pak_id_counter > 255)
         fragmented_pak_id_counter = 0; // wrap it around at eight bits
   }

   // else datagram is shorter than 1024 bytes and doesn't need to be fragmented
   else
   {
      // compute pak ID from master ID and fragment number
      id_160 = normal_pak_id_counter;
      normal_pak_id_counter++; // increment the pak ID (one more is being sent)
      if (normal_pak_id_counter > 8190)
         normal_pak_id_counter = 5760; // wrap it around at thirteen bits, start value is 5760

      // build the header
      header_word = id_160 << 3;
      if (pak->flags & PAKFLAG_ACKREQUESTED)
      {
         header_word |= PAKHEADER_160_TYPE_ACKREQUEST;

         ack_160 = Ack160_FindByOriginalId (pak->id); // first find the original ack request ID
         ack_160->arrival_date = current_time; // remember arrival date
         ack_160->original_id = pak->id;
         ack_160->final_160_id = id_160;
      }

      // build the UDP datagram: pak header, signature, and data
      memcpy (datagram_160.bytes, &header_word, 2);
      memset (datagram_160.bytes + 2, 0, 4); // void signature for standalone datagrams
      memcpy (datagram_160.bytes + 2 + 4, pak->data, pak->data_size);
      datagram_160.size = pak->data_size + 2 + 4 + 1;

      // put back the 1.60 XOR crypto on the datagram and add its byte checksum
      Datagram_EnXOR_160 (&datagram_160, datagram_160.size - 1);

      // add this datagram to the list of datagrams to send
      if ((pak->flags & PAKFLAG_NEW_PAK) && (pak->flags & PAKFLAG_ACKREQUESTED) && (ack_160 != NULL))
         Network_PrepareDatagram (&datagram_160, true, ack_160->original_id);
      else
         Network_PrepareDatagram (&datagram_160, false, 0);
   }

   return; // datagram list successfully built
}


void Proto160_BuildAckReplyDatagramsFromPak (pak_t *pak, int socket_from, address_t *address_to)
{
   // this function builds one or several ack reply datagrams ready to be sent over the network
   // from a dropped T4C pak

   unsigned short header_word;
   int fragment_number;

   // either the dropped pak is smaller than 1024 bytes and we send only one ack reply,
   // or it's longer and we have to send one ack reply for each fragment.

   // copy source socket and destination address
   datagram_160.our_socket = socket_from;
   memcpy (&datagram_160.peer_address, address_to, sizeof (address_t));

   // first prepare a blank datagram header
   memset (datagram_160.bytes, 0, 2);
   datagram_160.size = 2;

   // is the dropped pak a fragmented pak ?
   if (pak->data_size + 6 > 1024)
   {
      // for each of the sub-fragments...
      for (fragment_number = 0; fragment_number < (pak->data_size - 6) / 1024; fragment_number++)
      {
         // report the pak ID in the ack reply
         datagram_160.bytes[0] = ((unsigned char) fragment_number) << 3;
         datagram_160.bytes[1] = (unsigned char) pak->id;

         // add this datagram to the list of datagrams to send
         Network_PrepareDatagram (&datagram_160, false, 0);
      }
   }

   // else datagram is shorter than 1024 bytes and doesn't need to be fragmented
   else
   {
      // report the pak ID in the ack reply
      header_word = ((unsigned short) pak->id) << 3;
      memcpy (datagram_160.bytes, &header_word, 2);

      // add this datagram to the list of datagrams to send
      Network_PrepareDatagram (&datagram_160, false, 0);
   }

   return; // finished, ack reply datagram list is built
}


static void Pak_SwitchEncryptionOff_160 (pak_t *pak)
{
   // the 1.50 protocol cryptography uses extensively the standard C random number generator,
   // which is a VERY BAD idea, since its implementation may differ from system to system !!!

   char stack1[10];
   char stack2[10];
   unsigned char a;
   unsigned char c;
   char *edi;
   char *ebp;
   int index;
   unsigned int algo;

   // initialize the system's pseudo-random number generator from the seed given in the datagram
   // (they apparently swapped the bytes in an attempt to confuse the reverse-engineerers)
   srand (((unsigned long) ((unsigned char) pak->data[pak->data_size - 4]) << 24)
          | ((unsigned long) ((unsigned char) pak->data[pak->data_size - 1]) << 16)
          | ((unsigned long) ((unsigned char) pak->data[pak->data_size - 3]) << 8)
          | ((unsigned long) ((unsigned char) pak->data[pak->data_size - 2]) << 0));
   pak->data_size -= 4; // correct new data size (we strip the seed)

   // now generate the crypto tables for the given datagram length

   // stack sequences
   for (index = 0; index < 10; index++)
   {
      stack1[index] = (char) rand ();
      stack2[index] = (char) rand ();
   }

   // xor table
   for (index = 0; index < pak->data_size - 1; index++)
   {
      cryptotables_160.xor[index] = (unsigned char) stack2[rand () % 10];
      cryptotables_160.xor[index] *= (unsigned char) stack1[rand () % 10];
      cryptotables_160.xor[index] += rand ();
   }

   // offset & algo tables
   for (index = 0; index < pak->data_size - 1; index++)
   {
      cryptotables_160.offsets[index] = rand () % (pak->data_size - 1);
      if (cryptotables_160.offsets[index] == (unsigned int) index)
         cryptotables_160.offsets[index] = (index == 0 ? 1 : 0);

      cryptotables_160.algo[index] = rand () % 21;
   }

   // cryptographic tables are generated, now apply the algorithm
   for (index = pak->data_size - 2; index >= 0; index--)
   {
      algo = cryptotables_160.algo[index];
      ebp = &pak->data[cryptotables_160.offsets[index]];
      edi = &pak->data[index];

      a = *ebp;
      c = *edi;

      if      (algo == 0)  { *edi = ((a ^ c) & 0x0F) ^ c;  *ebp = ((a ^ c) & 0x0F) ^ a;  }
      else if (algo == 1)  { *edi = ((a ^ c) & 0x0F) ^ c;  *ebp = (a >> 4) | (c << 4);   }
      else if (algo == 2)  { *edi = (c >> 4) | (c << 4);   *ebp = (a >> 4) | (a << 4);   }
      else if (algo == 3)  { *edi = (a >> 4) | (c << 4);   *ebp = ((a ^ c) & 0x0F) ^ c;  }
      else if (algo == 4)  { *edi = (a & 0x0F) | (c << 4); *ebp = (a & 0xF0) | (c >> 4); }
      else if (algo == 5)  { *edi = (c & 0xF0) | (a >> 4); *ebp = (a << 4) | (c & 0x0F); }
      else if (algo == 6)  { *edi = (a >> 4) | (c << 4);   *ebp = (a << 4) | (c >> 4);   }
      else if (algo == 7)  { *edi = (c & 0xF0) | (a >> 4); *ebp = (a & 0x0F) | (c << 4); }
      else if (algo == 8)  { *edi = (a & 0x0F) | (c << 4); *ebp = (c & 0xF0) | (a >> 4); }
      else if (algo == 9)  { *edi = (a & 0xF0) | (c >> 4); *ebp = (a & 0x0F) | (c << 4); }
      else if (algo == 10) { *edi = (a << 4) | (c & 0x0F); *ebp = (a & 0xF0) | (c >> 4); }
      else if (algo == 11) { *edi = (a << 4) | (c >> 4);   *ebp = ((a ^ c) & 0x0F) ^ a;  }
      else if (algo == 12) { *edi = (a >> 4) | (a << 4);   *ebp = (c >> 4) | (c << 4);   }
      else if (algo == 13) { *edi = a;                     *ebp = c;                     }
      else if (algo == 14) { *edi = (a & 0xF0) | (c >> 4); *ebp = (a << 4) | (c & 0x0F); }
      else if (algo == 15) { *edi = ((a ^ c) & 0x0F) ^ a;  *ebp = ((a ^ c) & 0x0F) ^ c;  }
      else if (algo == 16) { *edi = a;                     *ebp = (c >> 4) | (c << 4);   }
      else if (algo == 17) { *edi = (a << 4) | (c & 0x0F); *ebp = (c & 0xF0) | (a >> 4); }
      else if (algo == 18) { *edi = (a << 4) | (c >> 4);   *ebp = (a >> 4) | (c << 4);   }
      else if (algo == 19) { *edi = (a >> 4) | (a << 4);   *ebp = c;                     }
      else if (algo == 20) { *edi = ((a ^ c) & 0x0F) ^ a;  *ebp = (a << 4) | (c >> 4);   }
   }

   // and finally, quadruple-XOR the data out
   for (index = pak->data_size - 2; index >= 0; index--)
   {
      if (index <= pak->data_size - 5)
      {
         pak->data[index + 0] ^= (cryptotables_160.xor[index] & 0x000000FF);
         pak->data[index + 1] ^= (cryptotables_160.xor[index] & 0x0000FF00) >> 8;
         pak->data[index + 2] ^= (cryptotables_160.xor[index] & 0x00FF0000) >> 16;
         pak->data[index + 3] ^= (cryptotables_160.xor[index] & 0xFF000000) >> 24;
      }
      else if (index == pak->data_size - 4)
      {
         pak->data[index + 0] ^= (cryptotables_160.xor[index] & 0x0000FF);
         pak->data[index + 1] ^= (cryptotables_160.xor[index] & 0x00FF00) >> 8;
         pak->data[index + 2] ^= (cryptotables_160.xor[index] & 0xFF0000) >> 16;
      }
      else if (index == pak->data_size - 3)
      {
         pak->data[index + 0] ^= (cryptotables_160.xor[index] & 0x00FF);
         pak->data[index + 1] ^= (cryptotables_160.xor[index] & 0xFF00) >> 8;
      }
      else if (index == pak->data_size - 2)
         pak->data[index] ^= (cryptotables_160.xor[index] & 0xFF);
   }

   return; // finished, pak is decrypted
}


static bool Datagram_DeXOR_160 (datagram_t *datagram, int length)
{
   // this function decrypts a standard 1.60 datagram. It's a simple XOR. The 1-byte checksum is
   // computed after the encrypted bytes and a padding character is added at the end to make it
   // reach zero by overflow.

   unsigned char checksum;
   int index;

   // for each byte of data...
   checksum = 0;
   for (index = 0; index < length; index++)
   {
      checksum += datagram->bytes[index]; // compute checksum at the same time with the crypted data
      datagram->bytes[index] ^= DATAGRAM_DECRYPT_160_MASK[index / 512]; // mask it out
   }

   if (checksum != 0)
      return (false); // if checksum is NOT null, then something went wrong

   return (true); // checksum has been verified to be null, everything went fine
}


static void Datagram_EnXOR_160 (datagram_t *datagram, int length)
{
   // this function encrypts a standard 1.60 datagram. It's a simple XOR. The 1-byte checksum is
   // computed after the encrypted bytes and a padding character is added at the end to make it
   // reach zero by overflow.

   unsigned char checksum;
   int index;

   // for each byte of the datagram...
   checksum = 0;
   for (index = 0; index < length; index++)
   {
      datagram->bytes[index] ^= DATAGRAM_DECRYPT_160_MASK[index / 512]; // mask it out with the second static data mask
      checksum -= datagram->bytes[index]; // compute checksum at the same time with the crypted data
   }
   datagram->bytes[length] = checksum; // add a padding byte to ensure checksum reaches 0

   return; // return 0 since we didn't use the 1.50 cryptography in this datagram
}


static ack_160_t *Ack160_FindByOriginalId (int original_id)
{
   // this function finds the right slot in the array of the ack reply datagrams corresponding
   // to the given ID. If no slot is found, it allocates and returns space for a new one.

   int index;

   // loop through all the ack datagrams we know and return the right one if we find it
   for (index = 0; index < ack_count_160; index++)
      if (acks_160[index].original_id == original_id)
         return (&acks_160[index]); // we found it

   // ack not found, check if one of the existing ones can be overwritten
   for (index = 0; index < ack_count_160; index++)
      if (acks_160[index].arrival_date + 30 < current_time)
      {
         memset (&acks_160[index], 0, sizeof (ack_160_t)); // yes, so clean it up
         return (&acks_160[index]); // and return its slot
      }

   // no suitable slot could be found, so we must allocate a new one
   acks_160 = (ack_160_t *) SAFE_realloc (acks_160, ack_count_160, ack_count_160 + 1, sizeof (ack_160_t), true);
   ack_count_160++; // we know now one ack request datagram more

   return (&acks_160[index]); // and return the slot we just allocated
}


static ack_160_t *Ack160_FindBy160Id (unsigned short id_160)
{
   // this function finds the right slot in the array of the ack reply datagrams corresponding
   // to the given ID. If no slot is found, it allocates and returns space for a new one.

   int index;

   // loop through all the ack datagrams we know and return the right one if we find it
   for (index = 0; index < ack_count_160; index++)
      if (acks_160[index].final_160_id == id_160)
         return (&acks_160[index]); // we found it

   // ack not found, check if one of the existing ones can be overwritten
   for (index = 0; index < ack_count_160; index++)
      if (acks_160[index].arrival_date + 30 < current_time)
      {
         memset (&acks_160[index], 0, sizeof (ack_160_t)); // yes, so clean it up
         return (&acks_160[index]); // and return its slot
      }

   // no suitable slot could be found, so we must allocate a new one
   acks_160 = (ack_160_t *) SAFE_realloc (acks_160, ack_count_160, ack_count_160 + 1, sizeof (ack_160_t), true);
   ack_count_160++; // we know now one ack request datagram more

   return (&acks_160[index]); // and return the slot we just allocated
}


static unsigned short CRC16_160 (unsigned char *data, int length)
{
   // this function computes the 16-bit CRC over an array of bytes pointed to by data

   unsigned short crc;
   unsigned char index;
   unsigned char bit;
   unsigned char parity;

   crc = 0; // start with all bits low

   // for each byte of data...
   for (index = 0; index < length; index++)
   {    
      crc ^= data[index]; // XOR CRC with byte at index

      // for each bit of CRC
      for (bit = 0; bit < 8; bit++)
      {
         parity = (unsigned char) crc; // remember its value for the parity check
         crc >>= 1; // right-shift the CRC
         if (parity % 2)
            crc ^= 0xa001; // if CRC was odd, XOR it with the CCITT polynomial (0xA001)
      }
   }

   return (crc); // and return the computed 16-bit CRC
}
