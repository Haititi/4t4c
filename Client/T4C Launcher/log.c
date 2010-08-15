// log.c

#include "client.h"


// local definitions
#define IS_SERVER_PAK(x) ((pak_type == x) && (pak->flags & PAKFLAG_SERVER_PAK))
#define IS_CLIENT_PAK(x) ((pak_type == x) && (pak->flags & PAKFLAG_CLIENT_PAK))


// global variables for internal use only
static FILE *logfile_fp = NULL;
static char log_string_header[64];
static char log_string[MAX_STRING16];
static int log_string_len;


// function prototypes for internal use only
static void WritePakDetails (pak_t *pak);
static void WriteLineHeader (void);


void Log_Text (const char *logfile, const char *fmt, ...)
{
   // this function logs a message to the message log file.

   // THE T4C LOG STRING FORMAT IS \nSTRING (NEWLINE BEFORE DATA). This function converts
   // the standard string format STRING\n to T4C's format automatically. You do *NOT* have to
   // format your strings in the T4C format when calling these logging functions. Just do it
   // the usual way.

   va_list argptr;
   int index;

   // do we want no logging at all ?
   if (log_level == 0)
      return; // if so, just return

   // first off, open the log file for appending in ASCII mode
   if (fopen_s (&logfile_fp, logfile, "a") != 0)
      return; // don't log anything if we're unable to open the log file

   WriteLineHeader (); // prepare line header with current date and time

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   SAFE_vsnprintf (log_string, sizeof (log_string), fmt, argptr);
   va_end (argptr);

   index = strlen (log_string) - 1;
   if (log_string[index] == '\n')
      log_string[index] = 0; // if there's a carriage return at the end of the line, chop it

   fputs (log_string_header, logfile_fp); // write line header
   fputs (log_string, logfile_fp); // write line data
   fclose (logfile_fp); // close log file
   return; // and return
}


void Log_Hex (const char *logfile, const unsigned char *data, int data_size, const char *fmt, ...)
{
   // this function logs hexadecimal data to the message log file.

   // THE T4C LOG STRING FORMAT IS \nSTRING (NEWLINE BEFORE DATA). This function converts
   // the standard string format STRING\n to T4C's format automatically. You do *NOT* have to
   // format your strings in the T4C format when calling these logging functions. Just do it
   // the usual way.

   char format[256];
   va_list argptr;
   int index;
   int i;

   // do we want no logging at all ?
   if (log_level == 0)
      return; // if so, just return

   // first off, open the log file for appending in ASCII mode
   if (fopen_s (&logfile_fp, logfile, "a") != 0)
      return; // don't log anything if we're unable to open the log file

   WriteLineHeader (); // prepare line header with current date and time

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   SAFE_vsnprintf (log_string, sizeof (log_string), fmt, argptr);
   va_end (argptr);

   index = strlen (log_string) - 1;
   if (log_string[index] == '\n')
      log_string[index] = 0; // if there's a carriage return at the end of the line, chop it

   fputs (log_string_header, logfile_fp); // write line header
   fputs (log_string, logfile_fp); // write line data

   // now print the datagram details, starting with the length
   fprintf (logfile_fp, "\n        Data size: %d", data_size);

   // for each row of 16 bytes of data...
   for (index = 0; index < data_size; index += 16)
   {
      SAFE_snprintf (log_string, sizeof (log_string), "\n        %05d        ", index); // print array address of row
      for (i = 0; i < 16; i++)
         if (index + i < data_size)
         {
            SAFE_snprintf (format, sizeof (format), " %02X", data[index + i]);
            SAFE_strncatf (log_string, sizeof (log_string), format); // if row contains data, print data as hex bytes
         }
         else
            SAFE_strncatf (log_string, sizeof (log_string), "   "); // else fill the space with blanks
      SAFE_strncatf (log_string, sizeof (log_string), "        ");
      for (i = 0; i < 16; i++)
         if (index + i < data_size)
         {
            SAFE_snprintf (format, sizeof (format), "%c", (isprint (data[index + i]) ? (data[index + i] != '%' ? data[index + i] : '.') : '.'));
            SAFE_strncatf (log_string, sizeof (log_string), format); // now if row contains data, print data as ASCII
         }
         else
            SAFE_strncatf (log_string, sizeof (log_string), " "); // else fill the space with blanks
      fprintf (logfile_fp, log_string);
   }

   fclose (logfile_fp); // close log file
   return; // and return
}


void Log_SQL (const char *logfile, int sqlcode, const char *fmt, ...)
{
   // this function logs a SQL error details to the message log file

   // THE T4C LOG STRING FORMAT IS \nSTRING (NEWLINE BEFORE DATA). This function converts
   // the standard string format STRING\n to T4C's format automatically. You do *NOT* have to
   // format your strings in the T4C format when calling these logging functions. Just do it
   // the usual way.

   va_list argptr;
   int index;

   // do we want no logging at all ?
   if (log_level == 0)
      return; // if so, just return

   // first off, open the log file for appending in ASCII mode
   if (fopen_s (&logfile_fp, logfile, "a") != 0)
      return; // don't log anything if we're unable to open the log file

   WriteLineHeader (); // prepare line header with current date and time

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   SAFE_vsnprintf (log_string, sizeof (log_string), fmt, argptr);
   va_end (argptr);

   index = strlen (log_string) - 1;
   if (log_string[index] == '\n')
      log_string[index] = 0; // if there's a carriage return at the end of the line, chop it

   fputs (log_string_header, logfile_fp); // write line header
   fputs (log_string, logfile_fp); // write line data

   // now analyze the SQL error
   fprintf (logfile_fp, "\n        SQL code: %d", sqlcode);

   // success codes
   if (sqlcode == 0)
      fprintf (logfile_fp, "\n        message text: Successful");

   // error codes
   else if (sqlcode == -102)
      fprintf (logfile_fp, "\n        message text: String constant is too long.");
   else if (sqlcode == -117)
      fprintf (logfile_fp, "\n        message text: The number of values in the INSERT does not match the number of columns.");
   else if (sqlcode == -180)
      fprintf (logfile_fp, "\n        message text: Bad data in Date/Time/Timestamp.");
   else if (sqlcode == -181)
      fprintf (logfile_fp, "\n        message text: Bad data in Date/Time/Timestamp.");
   else if (sqlcode == -199)
      fprintf (logfile_fp, "\n        message text: Illegal use of the specified keyword.");
   else if (sqlcode == -204)
      fprintf (logfile_fp, "\n        message text: Object not defined to DB2.");
   else if (sqlcode == -205)
      fprintf (logfile_fp, "\n        message text: Column name not in table.");
   else if (sqlcode == -206)
      fprintf (logfile_fp, "\n        message text: Column does not exist in any table of the SELECT.");
   else if (sqlcode == -216)
      fprintf (logfile_fp, "\n        message text: Not the same number of expressions on both sides of the comparison in a SELECT.");
   else if (sqlcode == -224)
      fprintf (logfile_fp, "\n        message text: FETCH cannot make an INSENSITIVE cursor SENSITIVE.");
   else if (sqlcode == -229)
      fprintf (logfile_fp, "\n        message text: The locale specified in a SET LOCALE statement was not found.");
   else if (sqlcode == -305)
      fprintf (logfile_fp, "\n        message text: Null indicator needed.");
   else if (sqlcode == -311)
      fprintf (logfile_fp, "\n        message text: Varchar, insert or update. -LEN field with the right data length not set.");
   else if (sqlcode == -482)
      fprintf (logfile_fp, "\n        message text: The procedure returned no locators.");
   else if (sqlcode == -501)
      fprintf (logfile_fp, "\n        message text: Cursor not open on FETCH.");
   else if (sqlcode == -502)
      fprintf (logfile_fp, "\n        message text: Opening cursor that is already open.");
   else if (sqlcode == -503)
      fprintf (logfile_fp, "\n        message text: Updating column needs to be specified. ");
   else if (sqlcode == -530)
      fprintf (logfile_fp, "\n        message text: Referential integrity preventing the INSERT/UPDATE");
   else if (sqlcode == -532)
      fprintf (logfile_fp, "\n        message text: Referential integrity (DELETE RESTRICT rule) preventing the DELETE.");
   else if (sqlcode == -536)
      fprintf (logfile_fp, "\n        message text: Referential integrity (DELETE RESTRICT rule) preventing the DELETE.");
   else if (sqlcode == -545)
      fprintf (logfile_fp, "\n        message text: Check constraint preventing the INSERT/UPDATE.");
   else if (sqlcode == -747)
      fprintf (logfile_fp, "\n        message text: The table is not available.");
   else if (sqlcode == -803)
      fprintf (logfile_fp, "\n        message text: Duplicate key on insert or update.");
   else if (sqlcode == -805)
      fprintf (logfile_fp, "\n        message text: DBRM or package not found in plan.");
   else if (sqlcode == -811)
      fprintf (logfile_fp, "\n        message text: More than one row retrieved in SELECT INTO.");
   else if (sqlcode == -818)
      fprintf (logfile_fp, "\n        message text: Plan and program: timestamp mismatch.");
   else if (sqlcode == -904)
      fprintf (logfile_fp, "\n        message text: Unavailable resource. Someone else is locking your data.");
   else if (sqlcode == -911)
      fprintf (logfile_fp, "\n        message text: Deadlock or timeout. Rollback has been done.");
   else if (sqlcode == -913)
      fprintf (logfile_fp, "\n        message text: Deadlock or timeout. No rollback.");
   else if (sqlcode == -922)
      fprintf (logfile_fp, "\n        message text: Authorization needed.");
   else if (sqlcode == -927)
      fprintf (logfile_fp, "\n        message text: The language interface was called but no connection had been made.");

   // warning codes
   else if (sqlcode == 100)
      fprintf (logfile_fp, "\n        message text: Row not found or end of cursor.");
   else if (sqlcode == 222)
      fprintf (logfile_fp, "\n        message text: Trying to fetch a row within a DELETE statement.");
   else if (sqlcode == 223)
      fprintf (logfile_fp, "\n        message text: Trying to fetch a row within an UPDATE statement.");
   else if (sqlcode == 231)
      fprintf (logfile_fp, "\n        message text: FETCH after a BEFORE or AFTER but not on a valid row. ");
   else if (sqlcode == 304)
      fprintf (logfile_fp, "\n        message text: Value cannot be assigned to this host variable because it is out of range.");
   else if (sqlcode == 802)
      fprintf (logfile_fp, "\n        message text: The null indicator was set to -2 as an arithmetic statement didn't work.");

   // unhandled code
   else
      fprintf (logfile_fp, "\n        unhandled SQL code, please look it up on the internet.");

   fclose (logfile_fp);
   return; // and return
}


void Log_Datagram (const char *logfile, datagram_t *datagram, const char *fmt, ...)
{
   // this function logs a datagram to the message log file

   // THE T4C LOG STRING FORMAT IS \nSTRING (NEWLINE BEFORE DATA). This function converts
   // the standard string format STRING\n to T4C's format automatically. You do *NOT* have to
   // format your strings in the T4C format when calling these logging functions. Just do it
   // the usual way.

   va_list argptr;
   char format[256];
   unsigned char data;
   int index;
   int i;

   // do we want no logging at all ?
   if (log_level == 0)
      return; // if so, just return

   // first off, open the log file for appending in ASCII mode
   if (fopen_s (&logfile_fp, logfile, "a") != 0)
      return; // don't log anything if we're unable to open the log file

   WriteLineHeader (); // prepare line header with current date and time

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   SAFE_vsnprintf (log_string, sizeof (log_string), fmt, argptr);
   va_end (argptr);

   index = strlen (log_string) - 1;
   if (log_string[index] == '\n')
      log_string[index] = 0; // if there's a carriage return at the end of the line, chop it

   fputs (log_string_header, logfile_fp); // write line header
   fputs (log_string, logfile_fp); // write line data

   // now print the datagram details, starting with the length
   fprintf (logfile_fp, "\n        Datagram length: %d", datagram->size);

   // for each row of 16 bytes of data...
   for (index = 0; index < datagram->size; index += 16)
   {
      SAFE_snprintf (log_string, sizeof (log_string), "\n        %05d        ", index); // print array address of row
      for (i = 0; i < 16; i++)
         if (index + i < datagram->size)
         {
            data = datagram->bytes[index + i];
            SAFE_snprintf (format, sizeof (format), " %02X", data);
            SAFE_strncatf (log_string, sizeof (log_string), format); // if row contains data, print data as hex bytes
         }
         else
            SAFE_strncatf (log_string, sizeof (log_string), "   "); // else fill the space with blanks
      SAFE_strncatf (log_string, sizeof (log_string), "        ");
      for (i = 0; i < 16; i++)
         if (index + i < datagram->size)
         {
            data = datagram->bytes[index + i];
            SAFE_snprintf (format, sizeof (format), "%c", (isprint (data) ? (data != '%' ? data : '.') : '.'));
            SAFE_strncatf (log_string, sizeof (log_string), format); // now if row contains data, print data as ASCII
         }
         else
            SAFE_strncatf (log_string, sizeof (log_string), " "); // else fill the space with blanks
      fprintf (logfile_fp, log_string);
   }

   fclose (logfile_fp);
   return; // and return
}


void Log_Pak (const char *logfile, pak_t *pak, const char *fmt, ...)
{
   // this function logs a pak to the message log file

   // THE T4C LOG STRING FORMAT IS \nSTRING (NEWLINE BEFORE DATA). This function converts
   // the standard string format STRING\n to T4C's format automatically. You do *NOT* have to
   // format your strings in the T4C format when calling these logging functions. Just do it
   // the usual way.

   va_list argptr;
   char format[256];
   unsigned char data;
   int index;
   int i;

   // do we want no logging at all ?
   if (log_level == 0)
      return; // if so, just return

   // first off, open the log file for appending in ASCII mode
   if (fopen_s (&logfile_fp, logfile, "a") != 0)
      return; // don't log anything if we're unable to open the log file

   WriteLineHeader (); // prepare line header with current date and time

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   SAFE_vsnprintf (log_string, sizeof (log_string), fmt, argptr);
   va_end (argptr);

   index = strlen (log_string) - 1;
   if (log_string[index] == '\n')
      log_string[index] = 0; // if there's a carriage return at the end of the line, chop it

   fputs (log_string_header, logfile_fp); // write line header
   fputs (log_string, logfile_fp); // write line data

   // if this pak wears the newly generated pak flag, notify about it
   if (pak->flags & PAKFLAG_NEW_PAK)
      fprintf (logfile_fp, "\n        Notice: this pak has been generated.");

   // if this pak wears the ack requested pak flag, notify about it
   if (pak->flags & PAKFLAG_ACKREQUESTED)
      fprintf (logfile_fp, "\n        Notice: an acknowledgment for this pak is requested.");

   // if this pak wears the ack requested pak flag, notify about it
   if (pak->flags & PAKFLAG_ACKREPLY)
      fprintf (logfile_fp, "\n        Notice: this pak is an acknowledgment reply.");

   // does this pak contain data ?
   if (pak->data_size > 0)
   {
      WritePakDetails (pak); // log pak details

      fprintf (logfile_fp, "\n");
      fprintf (logfile_fp, "\n        Raw data: %d bytes", pak->data_size); // print the data size

      // for each row of 16 bytes of data...
      for (index = 0; index < pak->data_size; index += 16)
      {
         SAFE_snprintf (log_string, sizeof (log_string), "\n        %05d        ", index);
         for (i = 0; i < 16; i++)
            if (index + i < pak->data_size)
            {
               data = Pak_ReadInt8 (pak, index + i);
               SAFE_snprintf (format, sizeof (format), " %02X", data);
               SAFE_strncatf (log_string, sizeof (log_string), format); // if row contains data, print data as hex bytes
            }
            else
               SAFE_strncatf (log_string, sizeof (log_string), "   "); // else fill the space with blanks
         SAFE_strncatf (log_string, sizeof (log_string), "        ");
         for (i = 0; i < 16; i++)
            if (index + i < pak->data_size)
            {
               data = Pak_ReadInt8 (pak, index + i);
               SAFE_snprintf (format, sizeof (format), "%c", (isprint (data) ? (data != '%' ? data : '.') : '.'));
               SAFE_strncatf (log_string, sizeof (log_string), format); // now if row contains data, print data as ASCII
            }
            else
               SAFE_strncatf (log_string, sizeof (log_string), " "); // else fill the space with blank
         fprintf (logfile_fp, log_string);
      }
   }

   // do a line break
   fprintf (logfile_fp, "\n\n");

   fclose (logfile_fp);

   return; // and return
}


static void WritePakDetails (pak_t *pak)
{
   // this function logs the full details of a pak (composition, variable types and data) in the
   // current log file.

   int16 pak_type;
   int loop_count;
   int base;
   int i;
   int j;

   fprintf (logfile_fp, "\n");
   fprintf (logfile_fp, "\n        Resolved data: %d bytes", pak->data_size); // print what we're about to do

   // read the type of pak it is
   pak_type = Pak_ReadInt16 (pak, 0);

   // given pak type, display the type of data it is
   if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_N))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_N", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_UpdateCoordinates))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UpdateCoordinates", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int16 skin_id = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 8));
      fprintf (logfile_fp, "\n                int8 light_percentage = %d;", Pak_ReadInt8 (pak, 12));
      fprintf (logfile_fp, "\n                int8 unit_type = %d; // (0:monster, 1:NPC, 2:player)", Pak_ReadInt8 (pak, 13));
      fprintf (logfile_fp, "\n                int8 health_percentage = %d;", Pak_ReadInt8 (pak, 14));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_NE))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_NE", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_E))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_E", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_SE))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_SE", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_S))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_S", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_SW))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_SW", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_W))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_W", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_NW))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_NW", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerMove_Stop))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerMove_Stop", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_SynchronizePlayerCoordinates))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_SynchronizePlayerCoordinates", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int16 world = %d;", Pak_ReadInt16 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_KeepAlive))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_KeepAlive (pong)", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_KeepAlive))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_KeepAlive (ping)", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_GetObject))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_GetObject", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 object_id = %d;", Pak_ReadInt32 (pak, 6));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_RemoveUnit))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_RemoveUnit", pak_type);
      fprintf (logfile_fp, "\n                int8 unknown = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 3));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_DepositObject))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_DepositObject", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 object_id = %d;", Pak_ReadInt32 (pak, 6));
      fprintf (logfile_fp, "\n                int32 item_count = %d;", Pak_ReadInt32 (pak, 10));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_SetUnitAppearance))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_SetUnitAppearance", pak_type);
      fprintf (logfile_fp, "\n                int16 skin_id = %d; // (referenced)", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 4));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PutPlayerInGame))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PutPlayerInGame", pak_type);
      Pak_ReadString8 (pak, 2, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 playername = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_PutPlayerInGame))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PutPlayerInGame", pak_type);
      fprintf (logfile_fp, "\n                int8 unknown1 = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int32 player_id = %d;", Pak_ReadInt32 (pak, 3));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 7));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 9));
      fprintf (logfile_fp, "\n                int16 world = %d;", Pak_ReadInt16 (pak, 11));
      fprintf (logfile_fp, "\n                int32 health = %d;", Pak_ReadInt32 (pak, 13));
      fprintf (logfile_fp, "\n                int32 max_health = %d;", Pak_ReadInt32 (pak, 17));
      fprintf (logfile_fp, "\n                int16 mana = %d;", Pak_ReadInt16 (pak, 21));
      fprintf (logfile_fp, "\n                int16 max_mana = %d;", Pak_ReadInt16 (pak, 23));
      fprintf (logfile_fp, "\n                int64 xp = %d;", Pak_ReadInt64 (pak, 25));
      fprintf (logfile_fp, "\n                int64 next_level_xp = %d;", Pak_ReadInt64 (pak, 33));
      fprintf (logfile_fp, "\n                int16 strength = %d;", Pak_ReadInt16 (pak, 41));
      fprintf (logfile_fp, "\n                int16 endurance = %d;", Pak_ReadInt16 (pak, 43));
      fprintf (logfile_fp, "\n                int16 dexterity = %d;", Pak_ReadInt16 (pak, 45));
      fprintf (logfile_fp, "\n                int16 willpower = %d; // (unused)", Pak_ReadInt16 (pak, 47));
      fprintf (logfile_fp, "\n                int16 wisdom = %d;", Pak_ReadInt16 (pak, 49));
      fprintf (logfile_fp, "\n                int16 intelligence = %d;", Pak_ReadInt16 (pak, 51));
      fprintf (logfile_fp, "\n                int16 luck = %d; // (unused)", Pak_ReadInt16 (pak, 53));
      fprintf (logfile_fp, "\n                int8 seconds = %d;", Pak_ReadInt8 (pak, 55));
      fprintf (logfile_fp, "\n                int8 minutes = %d;", Pak_ReadInt8 (pak, 56));
      fprintf (logfile_fp, "\n                int8 hour = %d;", Pak_ReadInt8 (pak, 57));
      fprintf (logfile_fp, "\n                int8 week = %d;", Pak_ReadInt8 (pak, 58));
      fprintf (logfile_fp, "\n                int8 day = %d;", Pak_ReadInt8 (pak, 59));
      fprintf (logfile_fp, "\n                int8 month = %d;", Pak_ReadInt8 (pak, 60));
      fprintf (logfile_fp, "\n                int16 year = %d;", Pak_ReadInt16 (pak, 61));
      fprintf (logfile_fp, "\n                int32 gold = %d;", Pak_ReadInt32 (pak, 63));
      fprintf (logfile_fp, "\n                int16 level = %d;", Pak_ReadInt16 (pak, 67));
      fprintf (logfile_fp, "\n                int64 base_level_xp = %d;", Pak_ReadInt64 (pak, 69));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_Login))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_Login", pak_type);
      base = 2;
      log_string_len = Pak_ReadString8 (pak, base, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 login = \"%s\";", log_string);
      base += 1 + log_string_len;
      log_string_len = Pak_ReadString8 (pak, base, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 password = \"%s\";", log_string);
      base += 1 + log_string_len;
      fprintf (logfile_fp, "\n                int16 client_version_number = %d;", Pak_ReadInt16 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 unknown = %d;", Pak_ReadInt16 (pak, base + 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_Login))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_Login", pak_type);
      fprintf (logfile_fp, "\n                int8 status = %d; // (0:welcome, 1:error, 2:already logged in)", Pak_ReadInt8 (pak, 2));
      Pak_ReadString16 (pak, 3, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string message = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_DeletePlayer))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_DeletePlayer", pak_type);
      Pak_ReadString8 (pak, 2, log_string, (int8) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 playername = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_DeletePlayer))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_DeletePlayer", pak_type);
      fprintf (logfile_fp, "\n                int8 status = %d; // (0:no error, 1:error)", Pak_ReadInt8 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_SendPeriphericObjects))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_SendPeriphericObjects", pak_type);
      fprintf (logfile_fp, "\n                int8 unknown = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 3));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 5));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_SendPeriphericObjects))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_SendPeriphericObjects", pak_type);
      loop_count = Pak_ReadInt16 (pak, 2);
      fprintf (logfile_fp, "\n                int16 unit_count = %d;", loop_count);
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 x_coord = %d;", Pak_ReadInt16 (pak, 4 + (i * 13) + 0));
         fprintf (logfile_fp, "\n                        int16 y_coord = %d;", Pak_ReadInt16 (pak, 4 + (i * 13) + 2));
         fprintf (logfile_fp, "\n                        int16 skin_id = %d;", Pak_ReadInt16 (pak, 4 + (i * 13) + 4));
         fprintf (logfile_fp, "\n                        int32 unit_id = %d;", Pak_ReadInt32 (pak, 4 + (i * 13) + 6));
         fprintf (logfile_fp, "\n                        int8 light_percentage = %d;", Pak_ReadInt8 (pak, 4 + (i * 13) + 10));
         fprintf (logfile_fp, "\n                        int8 unit_type = %d; // (0:monster, 1:npc, 2:player)", Pak_ReadInt8 (pak, 4 + (i * 13) + 11));
         fprintf (logfile_fp, "\n                        int8 health_percentage = %d;", Pak_ReadInt8 (pak, 4 + (i * 13) + 12));
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_BackpackItems))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_BackpackItems", pak_type);
      fprintf (logfile_fp, "\n                int16 unknown = %d;", Pak_ReadInt16 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_BackpackItems))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_BackpackItems", pak_type);
      fprintf (logfile_fp, "\n                int8 popup_backpack = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int32 player_id = %d;", Pak_ReadInt32 (pak, 3));
      loop_count = Pak_ReadInt16 (pak, 7);
      fprintf (logfile_fp, "\n                int16 item_count = %d;", loop_count);
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 icon_id = %d;", Pak_ReadInt16 (pak, 9 + (i * 16) + 0));
         fprintf (logfile_fp, "\n                        int32 unit_id = %d;", Pak_ReadInt32 (pak, 9 + (i * 16) + 2));
         fprintf (logfile_fp, "\n                        int16 object_id = %d;", Pak_ReadInt16 (pak, 9 + (i * 16) + 6));
         fprintf (logfile_fp, "\n                        int32 quantity = %d;", Pak_ReadInt32 (pak, 9 + (i * 16) + 8));
         fprintf (logfile_fp, "\n                        int32 number_of_charges = %d;", Pak_ReadInt32 (pak, 9 + (i * 16) + 12));
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_EquippedItems))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_EquippedItems", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_EquippedItems))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_EquippedItems", pak_type);
      fprintf (logfile_fp, "\n                int8 unknown = %d;", Pak_ReadInt8 (pak, 2));
      base = 3; // block base address
      fprintf (logfile_fp, "\n                int32 body_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 body_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 body_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 body_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 body_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 body_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 hands_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 hands_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 hands_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 hands_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 hands_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 hands_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 head_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 head_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 head_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 head_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 head_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 head_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 legs_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 legs_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 legs_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 legs_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 legs_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 legs_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 bracers_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 bracers_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 bracers_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 bracers_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 bracers_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 bracers_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 neck_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 neck_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 neck_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 neck_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 neck_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 neck_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 rhand_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 rhand_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 rhand_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 rhand_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 rhand_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 rhand_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 lhand_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 lhand_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 lhand_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 lhand_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 lhand_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 lhand_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 rring_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 rring_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 rring_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 rring_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 rring_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 rring_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 lring_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 lring_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 lring_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 lring_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 lring_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 lring_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 belt_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 belt_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 belt_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 belt_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 belt_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 belt_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 back_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 back_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 back_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 back_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 back_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 back_itemname = \"%s\";", log_string);
      base += 2 + log_string_len; // block base address
      fprintf (logfile_fp, "\n                int32 feet_item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
      fprintf (logfile_fp, "\n                int16 feet_icon_id = %d;", Pak_ReadInt16 (pak, base + 4));
      fprintf (logfile_fp, "\n                int16 feet_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
      fprintf (logfile_fp, "\n                int16 feet_item_count = %d;", Pak_ReadInt16 (pak, base + 8));
      fprintf (logfile_fp, "\n                int32 feet_unknown = %d;", Pak_ReadInt32 (pak, base + 10));
      base += 14;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 feet_itemname = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ExitGame))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ExitGame", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_ServerShutdown))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ServerShutdown", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_EquipObject))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_EquipObject", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_UnequipObject))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UnequipObject", pak_type);
      fprintf (logfile_fp, "\n                int8 equipment_position = %d;", Pak_ReadInt8 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_UseObject))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UseObject", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 item_unit_id = %d;", Pak_ReadInt32 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_Attack))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_Attack", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_CreatePlayer))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_CreatePlayer", pak_type);
      fprintf (logfile_fp, "\n                int8 dexterity = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int8 endurance = %d;", Pak_ReadInt8 (pak, 3));
      fprintf (logfile_fp, "\n                int8 intelligence = %d;", Pak_ReadInt8 (pak, 4));
      fprintf (logfile_fp, "\n                int8 strength = %d;", Pak_ReadInt8 (pak, 5));
      fprintf (logfile_fp, "\n                int8 wisdom = %d;", Pak_ReadInt8 (pak, 6));
      fprintf (logfile_fp, "\n                int8 unknown = %d; // (probably gender)", Pak_ReadInt8 (pak, 7));
      Pak_ReadString8 (pak, 8, log_string, (int8) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 playername = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_RollDices))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_RollDices", pak_type);
      fprintf (logfile_fp, "\n                int8 unknown = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int8 dexterity = %d;", Pak_ReadInt8 (pak, 3));
      fprintf (logfile_fp, "\n                int8 endurance = %d;", Pak_ReadInt8 (pak, 4));
      fprintf (logfile_fp, "\n                int8 intelligence = %d;", Pak_ReadInt8 (pak, 5));
      fprintf (logfile_fp, "\n                int8 luck = %d; // (unused)", Pak_ReadInt8 (pak, 6));
      fprintf (logfile_fp, "\n                int8 strength = %d;", Pak_ReadInt8 (pak, 7));
      fprintf (logfile_fp, "\n                int8 willpower = %d; // (unused)", Pak_ReadInt8 (pak, 8));
      fprintf (logfile_fp, "\n                int8 wisdom = %d;", Pak_ReadInt8 (pak, 9));
      fprintf (logfile_fp, "\n                int32 health = %d;", Pak_ReadInt32 (pak, 10));
      fprintf (logfile_fp, "\n                int32 max_health = %d;", Pak_ReadInt32 (pak, 14));
      fprintf (logfile_fp, "\n                int16 mana = %d;", Pak_ReadInt16 (pak, 18));
      fprintf (logfile_fp, "\n                int16 max_mana = %d;", Pak_ReadInt16 (pak, 20));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PersonalPCList))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PersonalPCList", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_PersonalPCList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PersonalPCList", pak_type);
      loop_count = Pak_ReadInt8 (pak, 2);
      fprintf (logfile_fp, "\n                int8 character_count = %d;", loop_count);
      base = 3; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         log_string_len = Pak_ReadString8 (pak, base, log_string, (unsigned char) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string8 playername = \"%s\";", log_string);
         base += 1 + log_string_len;
         fprintf (logfile_fp, "\n                        int16 skin_id = %d;", Pak_ReadInt16 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int16 level = %d;", Pak_ReadInt16 (pak, base + 2));
         base += 4;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_IndirectTalk))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_IndirectTalk", pak_type);
      fprintf (logfile_fp, "\n                int32 player_id = %d; // player ID in database", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int8 unit_orientation = %d;", Pak_ReadInt8 (pak, 6));
      fprintf (logfile_fp, "\n                int32 text_color = 0x%08x;", Pak_ReadInt32 (pak, 7));
      Pak_ReadString16 (pak, 11, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }
   else if (IS_SERVER_PAK (PAK_SERVER_IndirectTalk))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_IndirectTalk", pak_type);
      fprintf (logfile_fp, "\n                int32 sender_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int8 sender_orientation = %d;", Pak_ReadInt8 (pak, 6));
      fprintf (logfile_fp, "\n                int32 text_color = 0x%08x;", Pak_ReadInt32 (pak, 7));
      fprintf (logfile_fp, "\n                int8 sender_type = %d; // (0:player, 1:NPC)", Pak_ReadInt8 (pak, 11));
      base = 12;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
      base += 2 + log_string_len;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_Shout))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_Shout", pak_type);
      base = 2;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 playername = \"%s\";", log_string);
      base += 2 + log_string_len;
      fprintf (logfile_fp, "\n                int32 text_color = 0x%08x;", Pak_ReadInt32 (pak, base));
      base += 4;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_Page))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_Page", pak_type);
      base = 2;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 target_playername = \"%s\";", log_string);
      base += 2 + log_string_len;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_Page))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_Page", pak_type);
      base = 2;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 sender_playername = \"%s\";", log_string);
      base += 2 + log_string_len;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_DirectedTalk))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_DirectedTalk", pak_type);
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt32 (pak, 6));
      fprintf (logfile_fp, "\n                int8 orientation = %d;", Pak_ReadInt8 (pak, 10));
      fprintf (logfile_fp, "\n                int32 text_color = 0x%08x;", Pak_ReadInt32 (pak, 11));
      base = 15;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_Reroll))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_Reroll", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_Reroll))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_Reroll", pak_type);
      fprintf (logfile_fp, "\n                int8 dexterity = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int8 endurance = %d;", Pak_ReadInt8 (pak, 3));
      fprintf (logfile_fp, "\n                int8 intelligence = %d;", Pak_ReadInt8 (pak, 4));
      fprintf (logfile_fp, "\n                int8 luck = %d; // (unused)", Pak_ReadInt8 (pak, 5));
      fprintf (logfile_fp, "\n                int8 strength = %d;", Pak_ReadInt8 (pak, 6));
      fprintf (logfile_fp, "\n                int8 willpower = %d; // (unused)", Pak_ReadInt8 (pak, 7));
      fprintf (logfile_fp, "\n                int8 wisdom = %d;", Pak_ReadInt8 (pak, 8));
      fprintf (logfile_fp, "\n                int32 health = %d;", Pak_ReadInt32 (pak, 9));
      fprintf (logfile_fp, "\n                int32 max_health = %d;", Pak_ReadInt32 (pak, 13));
      fprintf (logfile_fp, "\n                int16 mana = %d;", Pak_ReadInt16 (pak, 17));
      fprintf (logfile_fp, "\n                int16 max_mana = %d;", Pak_ReadInt16 (pak, 19));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_CastSpell))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_CastSpell", pak_type);
      fprintf (logfile_fp, "\n                int16 spell_id = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt32 (pak, 8));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_SpellCastOK))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_SpellCastOK", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_UpdateLife))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UpdateLife", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_UpdateLife))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UpdateLife", pak_type);
      fprintf (logfile_fp, "\n                int32 health = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 max_health = %d;", Pak_ReadInt32 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_BroadcastTextChange))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_BroadcastTextChange", pak_type);
      fprintf (logfile_fp, "\n                int32 npc_unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 start_line_number = %d;", Pak_ReadInt16 (pak, 6));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_BroadcastTextChange))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_BroadcastTextChange", pak_type);
      fprintf (logfile_fp, "\n                int32 npc_unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 start_line_number = %d;", Pak_ReadInt16 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_UnitName))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UnitName", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 8));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_UnitName))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UnitName", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      base = 6;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_BreakConversation))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_BreakConversation", pak_type);
      fprintf (logfile_fp, "\n                int32 npc_unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 npc_x_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 npc_y_coord = %d;", Pak_ReadInt16 (pak, 8));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_BreakConversation))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_BreakConversation", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_UpdateLevel))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UpdateLevel", pak_type);
      fprintf (logfile_fp, "\n                int16 level = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int64 xp_left_before_next_level = %d;", Pak_ReadInt64 (pak, 4));
      fprintf (logfile_fp, "\n                int32 health = %d;", Pak_ReadInt32 (pak, 12));
      fprintf (logfile_fp, "\n                int32 max_health = %d;", Pak_ReadInt32 (pak, 16));
      fprintf (logfile_fp, "\n                int16 mana = %d;", Pak_ReadInt16 (pak, 20));
      fprintf (logfile_fp, "\n                int16 max_level = %d;", Pak_ReadInt16 (pak, 22));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ReturnToMenu))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ReturnToMenu", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_ReturnToMenu))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ReturnToMenu", pak_type);
      fprintf (logfile_fp, "\n                int8 status = %d; // (0:OK, 1:error)", Pak_ReadInt8 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PersonalSkills))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PersonalSkills", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_PersonalSkills))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PersonalSkills", pak_type);
      loop_count = Pak_ReadInt16 (pak, 2);
      fprintf (logfile_fp, "\n                int16 skill_count = %d;", loop_count);
      base = 4; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 skill_id = %d;", Pak_ReadInt16 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int8 use = %d; // 0 = permanent, 1 = on trigger, 3 = specify target", Pak_ReadInt8 (pak, base + 2));
         fprintf (logfile_fp, "\n                        int16 value = %d;", Pak_ReadInt16 (pak, base + 3));
         fprintf (logfile_fp, "\n                        int16 boosted_value = %d;", Pak_ReadInt16 (pak, base + 5));
         base += 7;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 skillname = \"%s\";", log_string);
         base += 2 + log_string_len;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 description = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_TrainList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_TrainList", pak_type);
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt32 (pak, 6));
      loop_count = Pak_ReadInt16 (pak, 10);
      fprintf (logfile_fp, "\n                int16 skill_count = %d;", loop_count);
      base = 12; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 skill_id = %d;", Pak_ReadInt16 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int16 points_count = %d;", Pak_ReadInt16 (pak, base + 2));
         base += 4;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_TrainList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_TrainList", pak_type);
      fprintf (logfile_fp, "\n                int16 available_points = %d;", Pak_ReadInt16 (pak, 2));
      loop_count = Pak_ReadInt16 (pak, 4);
      fprintf (logfile_fp, "\n                int16 skill_count = %d;", loop_count);
      base = 6; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int8 status = %d; // (0:can't train this skill, 1:can train this skill)", Pak_ReadInt8 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int16 skill_id = %d;", Pak_ReadInt16 (pak, base + 1));
         fprintf (logfile_fp, "\n                        int32 current_skill_value = %d;", Pak_ReadInt32 (pak, base + 3));
         fprintf (logfile_fp, "\n                        int16 max_skill_value = %d;", Pak_ReadInt16 (pak, base + 7));
         fprintf (logfile_fp, "\n                        int16 price = %d;", Pak_ReadInt16 (pak, base + 9));
         base += 11;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 skillname = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_BuyList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_BuyList", pak_type);
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt32 (pak, 6));
      loop_count = Pak_ReadInt16 (pak, 10);
      fprintf (logfile_fp, "\n                int16 item_count = %d;", loop_count);
      base = 12; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 object_id = %d;", Pak_ReadInt16 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int16 item_count = %d;", Pak_ReadInt16 (pak, base + 2));
         base += 4;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_BuyList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_BuyList", pak_type);
      fprintf (logfile_fp, "\n                int32 player_gold = %d;", Pak_ReadInt32 (pak, 2));
      loop_count = Pak_ReadInt16 (pak, 6);
      fprintf (logfile_fp, "\n                int16 item_count = %d;", loop_count);
      base = 8; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 object_id = %d;", Pak_ReadInt16 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int16 icon_id = %d;", Pak_ReadInt16 (pak, base + 2));
         fprintf (logfile_fp, "\n                        int32 price = %d;", Pak_ReadInt32 (pak, base + 4));
         fprintf (logfile_fp, "\n                        int8 status = %d; // (0:can't buy this item, 1:can buy this item)", Pak_ReadInt8 (pak, base + 8));
         base += 9;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 itemname = \"%s\";", log_string);
         base += 2 + log_string_len;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 description = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_UseSkill))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UseSkill", pak_type);
      fprintf (logfile_fp, "\n                int16 skill_id = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt32 (pak, 8));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PersonalStats))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PersonalStats", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_PersonalStats))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PersonalStats", pak_type);
      fprintf (logfile_fp, "\n                int32 health = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int32 max_health = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 mana = %d;", Pak_ReadInt16 (pak, 10));
      fprintf (logfile_fp, "\n                int16 max_mana = %d;", Pak_ReadInt16 (pak, 12));
      fprintf (logfile_fp, "\n                int64 xp = %d;", Pak_ReadInt64 (pak, 14));
      fprintf (logfile_fp, "\n                int16 boosted armor = %d;", Pak_ReadInt16 (pak, 22));
      fprintf (logfile_fp, "\n                int16 armor = %d;", Pak_ReadInt16 (pak, 24));
      fprintf (logfile_fp, "\n                int16 boosted_strength = %d;", Pak_ReadInt16 (pak, 26));
      fprintf (logfile_fp, "\n                int16 boosted_endurance = %d;", Pak_ReadInt16 (pak, 28));
      fprintf (logfile_fp, "\n                int16 boosted_dexterity = %d;", Pak_ReadInt16 (pak, 30));
      fprintf (logfile_fp, "\n                int16 boosted_willpower = %d; // (unused)", Pak_ReadInt16 (pak, 32));
      fprintf (logfile_fp, "\n                int16 boosted_wisdom = %d;", Pak_ReadInt16 (pak, 34));
      fprintf (logfile_fp, "\n                int16 boosted_intelligence = %d;", Pak_ReadInt16 (pak, 36));
      fprintf (logfile_fp, "\n                int16 boosted_luck = %d; // (unused)", Pak_ReadInt16 (pak, 38));
      fprintf (logfile_fp, "\n                int16 stat_points = %d;", Pak_ReadInt16 (pak, 40));
      fprintf (logfile_fp, "\n                int16 strength = %d;", Pak_ReadInt16 (pak, 42));
      fprintf (logfile_fp, "\n                int16 endurance = %d;", Pak_ReadInt16 (pak, 44));
      fprintf (logfile_fp, "\n                int16 dexterity = %d;", Pak_ReadInt16 (pak, 46));
      fprintf (logfile_fp, "\n                int16 willpower = %d; // (unused)", Pak_ReadInt16 (pak, 48));
      fprintf (logfile_fp, "\n                int16 wisdom = %d;", Pak_ReadInt16 (pak, 50));
      fprintf (logfile_fp, "\n                int16 intelligence = %d;", Pak_ReadInt16 (pak, 52));
      fprintf (logfile_fp, "\n                int16 luck = %d; // (unused)", Pak_ReadInt16 (pak, 54));
      fprintf (logfile_fp, "\n                int16 level = %d;", Pak_ReadInt16 (pak, 56));
      fprintf (logfile_fp, "\n                int16 skill_points = %d;", Pak_ReadInt16 (pak, 58));
      fprintf (logfile_fp, "\n                int16 luggage = %d;", Pak_ReadInt16 (pak, 60));
      fprintf (logfile_fp, "\n                int16 max_luggage = %d;", Pak_ReadInt16 (pak, 62));
      fprintf (logfile_fp, "\n                int16 karma = %d;", Pak_ReadInt16 (pak, 64));
      fprintf (logfile_fp, "\n                int16 base_health = %d;", Pak_ReadInt16 (pak, 66));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_UpdateXP))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UpdateXP", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_UpdateXP))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UpdateXP", pak_type);
      fprintf (logfile_fp, "\n                int64 xp = %d;", Pak_ReadInt64 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_UpdateTime))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UpdateTime", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_UpdateTime))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UpdateTime", pak_type);
      fprintf (logfile_fp, "\n                int8 seconds = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int8 minutes = %d;", Pak_ReadInt8 (pak, 3));
      fprintf (logfile_fp, "\n                int8 hours = %d;", Pak_ReadInt8 (pak, 4));
      fprintf (logfile_fp, "\n                int8 week = %d;", Pak_ReadInt8 (pak, 5));
      fprintf (logfile_fp, "\n                int8 day = %d;", Pak_ReadInt8 (pak, 6));
      fprintf (logfile_fp, "\n                int8 month = %d;", Pak_ReadInt8 (pak, 7));
      fprintf (logfile_fp, "\n                int16 year = %d;", Pak_ReadInt8 (pak, 8));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_FromPreInGameToInGame))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_FromPreInGameToInGame", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_FromPreInGameToInGame))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_FromPreInGameToInGame", pak_type);
      i = Pak_ReadInt8 (pak, 2);
      fprintf (logfile_fp, "\n                int8 no_data = %d;", i);
      if (!i)
      {
         fprintf (logfile_fp, "\n                int16 unknown = %d;", Pak_ReadInt16 (pak, 3));
         loop_count = Pak_ReadInt16 (pak, 5);
         fprintf (logfile_fp, "\n                int16 unit_count = %d;", loop_count);
         base = 7; // block base address
         for (i = 0; i < min (loop_count, 10); i++)
         {
            fprintf (logfile_fp, "\n                // array index: %d", i);
            fprintf (logfile_fp, "\n                        int16 x_coord = %d;", Pak_ReadInt16 (pak, base + 0));
            fprintf (logfile_fp, "\n                        int16 y_coord = %d;", Pak_ReadInt16 (pak, base + 2));
            fprintf (logfile_fp, "\n                        int16 skin_id = %d;", Pak_ReadInt16 (pak, base + 4));
            fprintf (logfile_fp, "\n                        int32 unit_id = %d;", Pak_ReadInt32 (pak, base + 6));
            fprintf (logfile_fp, "\n                        int8 light_percentage = %d;", Pak_ReadInt8 (pak, base + 10));
            fprintf (logfile_fp, "\n                        int8 unit_type = %d; // (0:monster, 1:NPC, 2:player)", Pak_ReadInt8 (pak, base + 11));
            fprintf (logfile_fp, "\n                        int8 health_percentage = %d;", Pak_ReadInt8 (pak, base + 12));
            base += 13;
         }
         if (loop_count > 10)
            fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
      }
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_EnterChannel))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_EnterChannel", pak_type);
      base = 2;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 channelname = \"%s\";", log_string);
      base += 2 + log_string_len;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 password = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ChannelMessage))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ChannelMessage", pak_type);
      base = 2;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 channelname = \"%s\";", log_string);
      base += 2 + log_string_len;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ChannelMessage))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ChannelMessage", pak_type);
      base = 2;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 channelname = \"%s\";", log_string);
      base += 2 + log_string_len;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 playername = \"%s\";", log_string);
      base += 2 + log_string_len;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ChannelUsers))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ChannelUsers", pak_type);
      base = 2;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 channelname = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ChannelUsers))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ChannelUsers", pak_type);
      base = 2;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 channelname = \"%s\";", log_string);
      base += 2 + log_string_len;
      loop_count = Pak_ReadInt16 (pak, base);
      fprintf (logfile_fp, "\n                int16 player_count = %d;", loop_count);
      base += 2;
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 playername = \"%s\";", log_string);
         base += 2 + log_string_len;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 playertitle = \"%s\";", log_string);
         base += 2 + log_string_len;
         fprintf (logfile_fp, "\n                        int8 status = %d; // (0:has closed channel, 1:has opened channel)", Pak_ReadInt8 (pak, base));
         base++;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerList))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerList", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_PlayerList))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PlayerList", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_GetSkillStatPoints))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_GetSkillStatPoints", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_UpdateGold))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UpdateGold", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_UpdateGold))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UpdateGold", pak_type);
      fprintf (logfile_fp, "\n                int32 gold = %d;", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ViewGroundItemIndentContent))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ViewGroundItemIndentContent", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_TeachList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_TeachList", pak_type);
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt32 (pak, 6));

      for (i = 10; i < pak->data_size; i+= 2)
         fprintf (logfile_fp, "\n                int16 skill_id = %d;", Pak_ReadInt16 (pak, i));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_TeachList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_TeachList", pak_type);
      fprintf (logfile_fp, "\n                int16 player_skillpoints = %d;", Pak_ReadInt32 (pak, 2));
      loop_count = Pak_ReadInt16 (pak, 4);
      fprintf (logfile_fp, "\n                int16 item_count = %d;", loop_count);
      base = 6; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int8 status = %d; // (0:can't buy this item, 1:can buy this item)", Pak_ReadInt8 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int16 skill_id = %d;", Pak_ReadInt16 (pak, base + 1));
         fprintf (logfile_fp, "\n                        int32 price = %d;", Pak_ReadInt32 (pak, base + 3));
         base += 7;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 skillname = \"%s\";", log_string);
         base += 2 + log_string_len;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 description = \"%s\";", log_string);
         base += 2 + log_string_len;
         fprintf (logfile_fp, "\n                        int32 cost_in_points = %d;", Pak_ReadInt32 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int32 skill_icon_id = %d;", Pak_ReadInt32 (pak, base + 4));
         base += 8;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_SellList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_SellList", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int32 buyer_npc_id = %d;", Pak_ReadInt32 (pak, 6));

      for (i = 10; i < pak->data_size; i+= 8)
      {
         fprintf (logfile_fp, "\n                int32 item_unit_id = %d;", Pak_ReadInt32 (pak, i));
         fprintf (logfile_fp, "\n                int32 quantity = %d;", Pak_ReadInt32 (pak, i + 4));
      }
   }

   else if (IS_SERVER_PAK (PAK_SERVER_SellList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_SellList", pak_type);
      fprintf (logfile_fp, "\n                int32 player_gold = %d;", Pak_ReadInt32 (pak, 2));
      loop_count = Pak_ReadInt16 (pak, 6);
      fprintf (logfile_fp, "\n                int16 item_count = %d;", loop_count);
      base = 8; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int32 item_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int16 item_type_id = %d;", Pak_ReadInt16 (pak, base + 4));
         fprintf (logfile_fp, "\n                        int32 price = %d;", Pak_ReadInt32 (pak, base + 6));
         fprintf (logfile_fp, "\n                        int32 quantity = %d;", Pak_ReadInt32 (pak, base + 10));
         base += 14;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 itemname = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_TeleportPlayer))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_TeleportPlayer", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int16 world = %d;", Pak_ReadInt16 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_SendStatTrain))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_SendStatTrain", pak_type);
      fprintf (logfile_fp, "\n                int8 strength = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int8 endurance = %d;", Pak_ReadInt8 (pak, 3));
      fprintf (logfile_fp, "\n                int8 dexterity = %d;", Pak_ReadInt8 (pak, 4));
      fprintf (logfile_fp, "\n                int8 willpower = %d; // (unused)", Pak_ReadInt8 (pak, 5));
      fprintf (logfile_fp, "\n                int8 wisdom = %d;", Pak_ReadInt8 (pak, 6));
      fprintf (logfile_fp, "\n                int8 intelligence = %d;", Pak_ReadInt8 (pak, 7));
      fprintf (logfile_fp, "\n                int8 luck = %d; // (unused)", Pak_ReadInt8 (pak, 8));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ItemName))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ItemName", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ItemName))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ItemName", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
      base = 6;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 itemname = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_GetNearItems))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_GetNearItems", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_NoUnitInView))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_NoUnitInView", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_PersonalSpellList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PersonalSpellList", pak_type);
      fprintf (logfile_fp, "\n                int8 player_is_god = %d; // (1 if god)", Pak_ReadInt8 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_PersonalSpellList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PersonalSpellList", pak_type);
      fprintf (logfile_fp, "\n                int8 unknown = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int16 mana = %d;", Pak_ReadInt16 (pak, 3));
      fprintf (logfile_fp, "\n                int16 max_mana = %d;", Pak_ReadInt16 (pak, 5));
      loop_count = Pak_ReadInt16 (pak, 7);
      fprintf (logfile_fp, "\n                int16 spell_count = %d;", loop_count);
      base = 9; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);

         // sometimes, null words are inserted in the pak. WTF ???? oO All we can do is read them
         // till we get a valid spell ID again, for legitimate spell data follows right after.
         for (;;)
         {
            j = Pak_ReadInt16 (pak, base + 0); // get spell ID
            if (j != 0)
               break; // break at the first valid ID we find

            fprintf (logfile_fp, "\n                int16 padding_word = 0x0000; // unknown purpose !");
            base += 2; // else advance one word further

            if (base > pak->data_size - 2)
               break; // consistency check
         }

         if (base > pak->data_size - 2)
            break; // consistency check

         fprintf (logfile_fp, "\n                        int16 spell_id = %d;", j);
         fprintf (logfile_fp, "\n                        int8 target_type = %d;", Pak_ReadInt8 (pak, base + 2));
         fprintf (logfile_fp, "\n                        int16 mana_cost = %d;", Pak_ReadInt16 (pak, base + 3));
         fprintf (logfile_fp, "\n                        int16 unknown1 = %d;", Pak_ReadInt16 (pak, base + 5));
         fprintf (logfile_fp, "\n                        int16 unknown2 = %d;", Pak_ReadInt16 (pak, base + 7));
         fprintf (logfile_fp, "\n                        int16 unknown3 = %d;", Pak_ReadInt16 (pak, base + 9));
         fprintf (logfile_fp, "\n                        int16 unknown4 = %d;", Pak_ReadInt16 (pak, base + 11));
         fprintf (logfile_fp, "\n                        int16 unknown5 = %d;", Pak_ReadInt16 (pak, base + 13));
         fprintf (logfile_fp, "\n                        int16 unknown6 = %d;", Pak_ReadInt16 (pak, base + 15));
         fprintf (logfile_fp, "\n                        int16 spell_icon = %d;", Pak_ReadInt16 (pak, base + 17));
         base += 19;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 description = \"%s\";", log_string);
         base += 2 + log_string_len;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 spellname = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ServerMessage))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ServerMessage", pak_type);
      fprintf (logfile_fp, "\n                int32 unused = %d;", Pak_ReadInt32 (pak, 2));
      base = 6;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_CastingSpell))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_CastingSpell", pak_type);
      fprintf (logfile_fp, "\n                int16 effect_id = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int32 source_id = %d;", Pak_ReadInt32 (pak, 4));
      fprintf (logfile_fp, "\n                int32 target_id = %d;", Pak_ReadInt32 (pak, 8));
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 12));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 14));
      fprintf (logfile_fp, "\n                int16 source_x_coord = %d;", Pak_ReadInt16 (pak, 16));
      fprintf (logfile_fp, "\n                int16 source_y_coord = %d;", Pak_ReadInt16 (pak, 18));
      fprintf (logfile_fp, "\n                int32 created_unit_id = %d;", Pak_ReadInt32 (pak, 20));
      fprintf (logfile_fp, "\n                int32 unknown = %d;", Pak_ReadInt32 (pak, 24));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ServerInformation))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ServerInformation", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_ServerInformation))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ServerInformation", pak_type);
      fprintf (logfile_fp, "\n                int32 version = %d;", Pak_ReadInt32 (pak, 2));
      loop_count = Pak_ReadInt16 (pak, 6);
      fprintf (logfile_fp, "\n                int16 patchserver_count = %d;", loop_count);
      base = 8; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 port_number = %d;", Pak_ReadInt16 (pak, base + 0));
         base += 2;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 address = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_MessageOfTheDay))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_MessageOfTheDay", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_MessageOfTheDay))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_MessageOfTheDay", pak_type);
      base = 2;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_UpdateMana))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UpdateMana", pak_type);
      fprintf (logfile_fp, "\n                int16 mana = %d;", Pak_ReadInt16 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PuppetInformation))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PuppetInformation", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 8));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_PuppetInformation))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PuppetInformation", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 body_type_id = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 feet_type_id = %d;", Pak_ReadInt16 (pak, 8));
      fprintf (logfile_fp, "\n                int16 hands_type_id = %d;", Pak_ReadInt16 (pak, 10));
      fprintf (logfile_fp, "\n                int16 head_type_id = %d;", Pak_ReadInt16 (pak, 12));
      fprintf (logfile_fp, "\n                int16 legs_type_id = %d;", Pak_ReadInt16 (pak, 14));
      fprintf (logfile_fp, "\n                int16 weapon_type_id = %d;", Pak_ReadInt16 (pak, 16));
      fprintf (logfile_fp, "\n                int16 shield_type_id = %d;", Pak_ReadInt16 (pak, 18));
      fprintf (logfile_fp, "\n                int16 back_type_id = %d;", Pak_ReadInt16 (pak, 20));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_UnitAppearance))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UnitAppearance", pak_type);
      fprintf (logfile_fp, "\n                int16 skin_id = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 4));
      fprintf (logfile_fp, "\n                int8 light_percentage = %d;", Pak_ReadInt8 (pak, 8));
      fprintf (logfile_fp, "\n                int8 unit_type = %d; // (0:monster, 1:NPC, 2:player)", Pak_ReadInt8 (pak, 9));
      fprintf (logfile_fp, "\n                int8 health_percentage = %d;", Pak_ReadInt8 (pak, 10));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_UnitNotFound))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_UnitNotFound", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 in_reply_to_pak_type = %d;", Pak_ReadInt16 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_QueryUnitExistence))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_QueryUnitExistence", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 8));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_UseItemByAppearance))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_UseItemByAppearance", pak_type);
      fprintf (logfile_fp, "\n                int16 object_id = %d;", Pak_ReadInt16 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_NoMoreItem))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_NoMoreItem", pak_type);
      fprintf (logfile_fp, "\n                int16 item_id = %d;", Pak_ReadInt16 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_RemoveFromChatterChannel))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_RemoveFromChatterChannel", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_ChannelList))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ChannelList", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_ChannelList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ChannelList", pak_type);
      loop_count = Pak_ReadInt16 (pak, 2);
      fprintf (logfile_fp, "\n                int16 channel_count = %d;", loop_count);
      base = 4; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 channelname = \"%s\";", log_string);
         base += 2 + log_string_len;
         fprintf (logfile_fp, "\n                        int8 status = %d; // (0:closed, 1:open)", Pak_ReadInt8 (pak, base));
         base ++;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_CreateGroup))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_CreateGroup", pak_type);
      fprintf (logfile_fp, "\n                int8 unknown = %d;", Pak_ReadInt8 (pak, 2));
      loop_count = Pak_ReadInt16 (pak, 3);
      fprintf (logfile_fp, "\n                int16 player_count = %d;", loop_count);
      base = 5; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int32 player_unit_id = %d;", Pak_ReadInt32 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int16 player_level = %d;", Pak_ReadInt16 (pak, base + 4));
         fprintf (logfile_fp, "\n                        int16 health_percentage = %d;", Pak_ReadInt16 (pak, base + 6));
         fprintf (logfile_fp, "\n                        int8 is_leader = %d;", Pak_ReadInt8 (pak, base + 8));
         base += 9;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 playername = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_GroupInvite))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_GroupInvite", pak_type);
      fprintf (logfile_fp, "\n                int32 target_player_unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 8));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_GroupInvite))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_GroupInvite", pak_type);
      fprintf (logfile_fp, "\n                int32 player_unit_id = %d;", Pak_ReadInt32 (pak, 2));
      base = 6;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_GroupJoin))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_GroupJoin", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_GroupLeave))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_GroupLeave", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_GroupLeave))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_GroupLeave", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_GroupKick))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_GroupKick", pak_type);
      fprintf (logfile_fp, "\n                int32 player_unit_id = %d;", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_DeleteGroup))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_DeleteGroup", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_ActivateUserSpell))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ActivateUserSpell", pak_type);
      fprintf (logfile_fp, "\n                int32 spell_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 remaining_duration = %d; // in milliseconds", Pak_ReadInt32 (pak, 6));
      fprintf (logfile_fp, "\n                int32 total_duration = %d; // in milliseconds", Pak_ReadInt32 (pak, 10));
      fprintf (logfile_fp, "\n                int32 icon_id = %d;", Pak_ReadInt32 (pak, 14));
      base = 18;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 description = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_SpellOff))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_SpellOff", pak_type);
      fprintf (logfile_fp, "\n                int32 spell_id = %d;", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_JunkItems))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_JunkItems", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 item_count = %d;", Pak_ReadInt32 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ToggleChatterListening))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ToggleChatterListening", pak_type);
      base = 2;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 channelname = \"%s\";", log_string);
      base += 2 + log_string_len;
      fprintf (logfile_fp, "\n                int8 listening_enabled = %d;", Pak_ReadInt8 (pak, base));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_GroupMemberHealth))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_GroupMemberHealth", pak_type);
      fprintf (logfile_fp, "\n                int32 player_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 health_percentage = %d\n", Pak_ReadInt16 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ToggleGroupAutoSplit))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_ToggleGroupAutoSplit", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_ToggleGroupAutoSplit))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ToggleGroupAutoSplit", pak_type);
      fprintf (logfile_fp, "\n                int8 status = %d; // (0:disabled, 1:enabled)", Pak_ReadInt8 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_TogglePages))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_TogglePages", pak_type);
      fprintf (logfile_fp, "\n                int8 status = %d; // (0:pages disabled, 1:pages enabled)", Pak_ReadInt8 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PlayerExists))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PlayerExists", pak_type);
      base = 2;
      Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 playername = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_PlayerExists))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PlayerExists", pak_type);
      fprintf (logfile_fp, "\n                int8 status = %d; // (0:does not exist, 1:exists, 2:invalid name)", Pak_ReadInt8 (pak, 2));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_PatchServerInformation))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_PatchServerInformation", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_PatchServerInformation))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PatchServerInformation", pak_type);
      fprintf (logfile_fp, "\n                int32 server_version = %d;", Pak_ReadInt32 (pak, 2));
      base = 6;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 url = \"%s\";", log_string);
      base += 2 + log_string_len;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 filename = \"%s\";", log_string);
      base += 2 + log_string_len;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 login = \"%s\";", log_string);
      base += 2 + log_string_len;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 password = \"%s\";", log_string);
      base += 2 + log_string_len;
      fprintf (logfile_fp, "\n                int16 unknown1 = %d;", Pak_ReadInt16 (pak, base));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_CarriableWeight))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_CarriableWeight", pak_type);
      fprintf (logfile_fp, "\n                int32 current_weight = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 max_weight = %d;", Pak_ReadInt32 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_Rob))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_Rob", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_Rob))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_Rob", pak_type);
      fprintf (logfile_fp, "\n                int8 can_rob = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int32 player_id = %d;", Pak_ReadInt32 (pak, 3));
      base = 7;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 playername = \"%s\";", log_string);
      base += 2 + log_string_len;
      loop_count = Pak_ReadInt16 (pak, base);
      fprintf (logfile_fp, "\n                int16 player_count = %d;", loop_count);
      base += 2; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 item_skin_id = %d;", Pak_ReadInt16 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int32 item_unit_id = %d;", Pak_ReadInt32 (pak, base + 2));
         fprintf (logfile_fp, "\n                        int16 item_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
         fprintf (logfile_fp, "\n                        int32 quantity = %d;", Pak_ReadInt32 (pak, base + 8));
         base += 12;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 itemname = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_DispelRob))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_DispelRob", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_DispelRob))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_DispelRob", pak_type);
      fprintf (logfile_fp, "\n                int8 unknown = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int32 player_id = %d;", Pak_ReadInt32 (pak, 3));
      base = 7;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 playername = \"%s\";", log_string);
      base += 2 + log_string_len;
      loop_count = Pak_ReadInt16 (pak, base);
      fprintf (logfile_fp, "\n                int16 player_count = %d;", loop_count);
      base += 2; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int16 item_skin_id = %d;", Pak_ReadInt16 (pak, base + 0));
         fprintf (logfile_fp, "\n                        int32 item_unit_id = %d;", Pak_ReadInt32 (pak, base + 2));
         fprintf (logfile_fp, "\n                        int16 item_object_id = %d;", Pak_ReadInt16 (pak, base + 6));
         fprintf (logfile_fp, "\n                        int32 quantity = %d;", Pak_ReadInt32 (pak, base + 8));
         base += 12;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 itemname = \"%s\";", log_string);
         base += 2 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ArrowMissed))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ArrowMissed", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 8));
      fprintf (logfile_fp, "\n                int8 health_percentage = %d;", Pak_ReadInt8 (pak, 10));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ArrowSuccess))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ArrowSuccess", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt32 (pak, 6));
      fprintf (logfile_fp, "\n                int8 health_percentage = %d;", Pak_ReadInt8 (pak, 10));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ServerFlags))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ServerFlags", pak_type);
      fprintf (logfile_fp, "\n                int8 scripts_enabled = %d;", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int8 help_enabled = %d;", Pak_ReadInt8 (pak, 3));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_SeraphArrival))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_SeraphArrival", pak_type);
      fprintf (logfile_fp, "\n                int16 pak1_id = 0x%04x; // PAK_SERVER_PopupUnit", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 skin_id = %d;", Pak_ReadInt16 (pak, 8));
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 10));
      fprintf (logfile_fp, "\n                int8 light_percentage = %d;", Pak_ReadInt8 (pak, 14));
      fprintf (logfile_fp, "\n                int8 unit_type = %d;", Pak_ReadInt8 (pak, 15));
      fprintf (logfile_fp, "\n                int8 health_percentage = %d;", Pak_ReadInt8 (pak, 16));
      fprintf (logfile_fp, "\n                int16 pak2_id = 0x%04x; // PAK_SERVER_PuppetInformation", Pak_ReadInt16 (pak, 17));
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 19));
      fprintf (logfile_fp, "\n                int16 body_type_id = %d;", Pak_ReadInt16 (pak, 23));
      fprintf (logfile_fp, "\n                int16 feet_type_id = %d;", Pak_ReadInt16 (pak, 25));
      fprintf (logfile_fp, "\n                int16 hands_type_id = %d;", Pak_ReadInt16 (pak, 27));
      fprintf (logfile_fp, "\n                int16 head_type_id = %d;", Pak_ReadInt16 (pak, 29));
      fprintf (logfile_fp, "\n                int16 legs_type_id = %d;", Pak_ReadInt16 (pak, 31));
      fprintf (logfile_fp, "\n                int16 weapon_type_id = %d;", Pak_ReadInt16 (pak, 33));
      fprintf (logfile_fp, "\n                int16 shield_type_id = %d;", Pak_ReadInt16 (pak, 35));
      fprintf (logfile_fp, "\n                int16 back_type_id = %d;", Pak_ReadInt16 (pak, 37));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_AuthenticateVersion))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_AuthenticateVersion", pak_type);
      fprintf (logfile_fp, "\n                int32 version_number = %d;", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_AuthenticateVersion))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_AuthenticateVersion", pak_type);
      fprintf (logfile_fp, "\n                int32 status = %d; // (assumed) (0:client version unusable, 1: client version OK)", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ResetRegistryInventory))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ResetRegistryInventory", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_InfoMessage))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_InfoMessage", pak_type);
      fprintf (logfile_fp, "\n                int32 type = %d; // arbitrary message type", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 color = 0x%08x; // alpha blue green red format", Pak_ReadInt32 (pak, 6));
      Pak_ReadString16 (pak, 10, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_InfoMessage2))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_InfoMessage2", pak_type);
      fprintf (logfile_fp, "\n                int32 type = %d; // arbitrary message type", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 color = 0x%08x; // alpha blue green red format", Pak_ReadInt32 (pak, 6));
      Pak_ReadString16 (pak, 10, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 message = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_MaxCharactersPerAccount))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_MaxCharactersPerAccount", pak_type);
      fprintf (logfile_fp, "\n                int8 max_characters = %d; // max amount of characters allowed per account (usually 3)", Pak_ReadInt8 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ToggleWeather))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ToggleWeather", pak_type);
      fprintf (logfile_fp, "\n                int32 weather_type = %d; // 0 = clear, 1 = rain, 2 = snow, 3 = fog", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 enable = %d; // 0 = disable, 1 = enable", Pak_ReadInt16 (pak, 6));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_OpenURL))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_OpenURL", pak_type);
      Pak_ReadString16 (pak, 2, log_string, (int16) sizeof (log_string));
      fprintf (logfile_fp, "\n                string16 url_to_open = \"%s\"; // goes straight into a ShellExecute, prefixed with \"http://\"", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ChestItems))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_ChestItems", pak_type);

      loop_count = Pak_ReadInt16 (pak, 2);
      fprintf (logfile_fp, "\n                int16 item_count = %d;", loop_count);
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                int16 item_skin_id = %d;", Pak_ReadInt16 (pak, 4 + 16 * i));
         fprintf (logfile_fp, "\n                int32 item_unit_id = %d;", Pak_ReadInt32 (pak, 4 + 16 * i + 2));
         fprintf (logfile_fp, "\n                int16 item_object_id = %d;", Pak_ReadInt16 (pak, 4 + 16 * i + 6));
         fprintf (logfile_fp, "\n                int32 quantity = %d;", Pak_ReadInt32 (pak, 4 + 16 * i + 8));
         fprintf (logfile_fp, "\n                int32 number_of_charges = %d;", Pak_ReadInt32 (pak, 4 + 16 * i + 12));
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_DepositChest))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = %d; // PAK_CLIENT_DepositChest", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 item_count = %d;", Pak_ReadInt32 (pak, 6));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_WithdrawChest))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = %d; // PAK_CLIENT_WithdrawChest", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 item_count = %d;", Pak_ReadInt32 (pak, 6));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_OpenChest))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_OpenChest", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_CloseChest))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_CloseChest", pak_type);

   else if (IS_CLIENT_PAK (PAK_CLIENT_RequestExchange))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = %d; // PAK_CLIENT_RequestExchange", pak_type);
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 8));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_RequestExchange))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_RequestExchange", pak_type);
      fprintf (logfile_fp, "\n                int16 target_unit_id = %d;", Pak_ReadInt16 (pak, 2));

      loop_count = Pak_ReadInt16 (pak, 4);
      fprintf (logfile_fp, "\n                int16 item_count = %d;", loop_count);
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                int16 item_skin_id = %d;", Pak_ReadInt16 (pak, 6 + 16 * i));
         fprintf (logfile_fp, "\n                int32 item_unit_id = %d;", Pak_ReadInt32 (pak, 6 + 16 * i + 2));
         fprintf (logfile_fp, "\n                int16 item_object_id = %d;", Pak_ReadInt16 (pak, 6 + 16 * i + 6));
         fprintf (logfile_fp, "\n                int32 quantity = %d;", Pak_ReadInt32 (pak, 6 + 16 * i + 8));
         fprintf (logfile_fp, "\n                int32 number_of_charges = %d;", Pak_ReadInt32 (pak, 6 + 16 * i + 12));
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_ItemDescription))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = %d; // PAK_CLIENT_ItemDescription", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_ItemDescription))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = %d; // PAK_SERVER_ItemDescription", pak_type);
      j = Pak_ReadInt8 (pak, 2);
      fprintf (logfile_fp, "\n                int8 unit_type = %d; // 0 = item unit; 1 = non-unit; 2 = non-item unit", j);
      if (j == 0)
      {
         fprintf (logfile_fp, "\n                // data following only if unit_type = 0");
         base = 3;
         log_string_len = Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string16 item_name = \"%s\";", log_string);
         base += 2 + log_string_len;
         fprintf (logfile_fp, "\n                        int16 item_skin_id = %d;", Pak_ReadInt16 (pak, base));
         fprintf (logfile_fp, "\n                        int8 light_percentage = %d;", Pak_ReadInt8 (pak, base + 2));
         fprintf (logfile_fp, "\n                        int16 item_armor_class = %d;", Pak_ReadInt16 (pak, base + 3));
         fprintf (logfile_fp, "\n                        int16 dodge_malus = %d;", Pak_ReadInt16 (pak, base + 5));
         fprintf (logfile_fp, "\n                        int16 required_endurance = %d;", Pak_ReadInt16 (pak, base + 7));
         fprintf (logfile_fp, "\n                        int32 minimum_damage = %d;", Pak_ReadInt32 (pak, base + 9));
         fprintf (logfile_fp, "\n                        int32 maximum_damage = %d;", Pak_ReadInt32 (pak, base + 13));
         fprintf (logfile_fp, "\n                        int16 required_attack = %d;", Pak_ReadInt16 (pak, base + 17));
         fprintf (logfile_fp, "\n                        int16 required_strength = %d;", Pak_ReadInt16 (pak, base + 19));
         fprintf (logfile_fp, "\n                        int16 required_agility = %d;", Pak_ReadInt16 (pak, base + 21));
         fprintf (logfile_fp, "\n                        int16 required_wisdom = %d;", Pak_ReadInt16 (pak, base + 23));
         fprintf (logfile_fp, "\n                        int16 required_intellect = %d;", Pak_ReadInt16 (pak, base + 25));
         base += 27;
         loop_count = Pak_ReadInt16 (pak, base);
         fprintf (logfile_fp, "\n                int16 boost_count = %d;", loop_count);
         base += 2; // block base address
         for (i = 0; i < min (loop_count, 10); i++)
         {
            fprintf (logfile_fp, "\n                // array index: %d", i);
            fprintf (logfile_fp, "\n                        int8 boosted_stat = %d;", Pak_ReadInt8 (pak, base));
            fprintf (logfile_fp, "\n                        int32 minimum_boost = %d;", Pak_ReadInt32 (pak, base + 1));
            fprintf (logfile_fp, "\n                        int32 maximum_boost = %d;", Pak_ReadInt32 (pak, base + 5));
            base += 9;
         }
         if (loop_count > 10)
            fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
      }
   }

   else if (IS_SERVER_PAK (PAK_SERVER_AttackMissed))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_AttackMissed", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int8 light_percentage = %d;", Pak_ReadInt8 (pak, 10));
      fprintf (logfile_fp, "\n                int8 unit_type = %d;", Pak_ReadInt8 (pak, 11));
      fprintf (logfile_fp, "\n                int8 health_percentage = %d;", Pak_ReadInt8 (pak, 12));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 13));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 15));
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 17));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 19));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_AttackSuccess))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_AttackSuccess", pak_type);
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 target_unit_id = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 10));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 12));
      fprintf (logfile_fp, "\n                int16 target_x_coord = %d;", Pak_ReadInt16 (pak, 14));
      fprintf (logfile_fp, "\n                int16 target_y_coord = %d;", Pak_ReadInt16 (pak, 16));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_SkillSuccess))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_SkillSuccess", pak_type);
      fprintf (logfile_fp, "\n                int16 skill_id = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 value = %d;", Pak_ReadInt16 (pak, 4));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_PopupUnit))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_PopupUnit", pak_type);
      fprintf (logfile_fp, "\n                int16 x_coord = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 y_coord = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int16 skin_id = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int32 unit_id = %d;", Pak_ReadInt32 (pak, 8));
      fprintf (logfile_fp, "\n                int8 light_percentage = %d;", Pak_ReadInt8 (pak, 12));
      fprintf (logfile_fp, "\n                int8 unit_type = %d;", Pak_ReadInt8 (pak, 13));
      fprintf (logfile_fp, "\n                int8 health_percentage = %d;", Pak_ReadInt8 (pak, 14));
   }

   //////////////////////////////////////////////////////////////////////////////////////
   // AEGIR'S SPECIAL PAKS
   //////////////////////////////////////////////////////////////////////////////////////

   else if (IS_CLIENT_PAK (PAK_CLIENT_AegirSendRPDescription))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_AegirSendRPDescription", pak_type);
      base = 2;
      log_string_len = Pak_ReadString8 (pak, base, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 title = \"%s\";", log_string);
      base += 1 + log_string_len;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 description = \"%s\";", log_string);
   }

   else if (IS_SERVER_PAK (PAK_SERVER_AegirSendRPDescription))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_AegirSendRPDescription", pak_type);
      fprintf (logfile_fp, "\n                int8 is_own = %d; // ", Pak_ReadInt8 (pak, 2));
      fprintf (logfile_fp, "\n                int16 gender = %d;", Pak_ReadInt8 (pak, 3));
      base = 4;
      log_string_len = Pak_ReadString8 (pak, base, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 name = \"%s\";", log_string);
      base += 1 + log_string_len;
      log_string_len = Pak_ReadString8 (pak, base, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 title = \"%s\";", log_string);
      base += 1 + log_string_len;
      log_string_len = Pak_ReadString16 (pak, base, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 description = \"%s\";", log_string);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_AegirRequestMagicalPR))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_AegirRequestMagicalPR", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_AegirSendMagicalPR))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_AegirSendMagicalPR", pak_type);
      fprintf (logfile_fp, "\n                int16 fire_power = %d;", Pak_ReadInt16 (pak, 2));
      fprintf (logfile_fp, "\n                int16 water_power = %d;", Pak_ReadInt16 (pak, 4));
      fprintf (logfile_fp, "\n                int16 earth_power = %d;", Pak_ReadInt16 (pak, 6));
      fprintf (logfile_fp, "\n                int16 air_power = %d;", Pak_ReadInt16 (pak, 8));
      fprintf (logfile_fp, "\n                int16 light_power = %d;", Pak_ReadInt16 (pak, 10));
      fprintf (logfile_fp, "\n                int16 dark_power = %d;", Pak_ReadInt16 (pak, 12));
      fprintf (logfile_fp, "\n                int16 fire_resistance = %d;", Pak_ReadInt16 (pak, 14));
      fprintf (logfile_fp, "\n                int16 water_resistance = %d;", Pak_ReadInt16 (pak, 16));
      fprintf (logfile_fp, "\n                int16 earth_resistance = %d;", Pak_ReadInt16 (pak, 18));
      fprintf (logfile_fp, "\n                int16 air_resistance = %d;", Pak_ReadInt16 (pak, 20));
      fprintf (logfile_fp, "\n                int16 light_resistance = %d;", Pak_ReadInt16 (pak, 22));
      fprintf (logfile_fp, "\n                int16 dark_resistance = %d;", Pak_ReadInt16 (pak, 24));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_AegirShowFriends))
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_AegirShowFriends", pak_type);

   else if (IS_SERVER_PAK (PAK_SERVER_AegirFriendsList))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_AegirFriendsList", pak_type);

      base = 2;
      loop_count = Pak_ReadInt8 (pak, base);
      fprintf (logfile_fp, "\n                int16 friend_count = %d;", loop_count);
      base += 1; // block base address
      for (i = 0; i < min (loop_count, 10); i++)
      {
         fprintf (logfile_fp, "\n                // array index: %d", i);
         fprintf (logfile_fp, "\n                        int8 friend_status = %d;", Pak_ReadInt8 (pak, base));
         base += 1;
         log_string_len = Pak_ReadString8 (pak, base, log_string, (int8) sizeof (log_string));
         fprintf (logfile_fp, "\n                        string8 friend_name = \"%s\";", log_string);
         base += 1 + log_string_len;
      }
      if (loop_count > 10)
         fprintf (logfile_fp, "\n                // %d elements to follow (truncated table)", loop_count - 10);
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_AegirSendParchment))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_AegirSendParchment", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
      base = 6;
      log_string_len = Pak_ReadString8 (pak, base, log_string, (unsigned char) sizeof (log_string));
      fprintf (logfile_fp, "\n                string8 title = \"%s\";", log_string);
      base += 1 + log_string_len;
      fprintf (logfile_fp, "\n                int32 uncompressed_size = %d;", Pak_ReadInt32 (pak, base));
      base += 4;
      fprintf (logfile_fp, "\n                rawstring16 compressed_buffer[%d];", Pak_ReadString16 (pak, base, log_string, (int16) sizeof (log_string)));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_AegirSendParchment))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_AegirSendParchment", pak_type);
      fprintf (logfile_fp, "\n                int32 item_id = %d;", Pak_ReadInt32 (pak, 2));
      fprintf (logfile_fp, "\n                int32 uncompressed_size = %d;", Pak_ReadInt32 (pak, 6));
      fprintf (logfile_fp, "\n                rawstring16 compressed_buffer[%d];", Pak_ReadString16 (pak, 10, log_string, (int16) sizeof (log_string)));
   }

   else if (IS_CLIENT_PAK (PAK_CLIENT_160Challenge))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_CLIENT_160Challenge", pak_type);
      fprintf (logfile_fp, "\n                int16 crc16 = %d;", Pak_ReadInt16 (pak, 2));
   }

   else if (IS_SERVER_PAK (PAK_SERVER_160Response))
   {
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // PAK_SERVER_160Response", pak_type);
      fprintf (logfile_fp, "\n                int16 crc16 = %d;", Pak_ReadInt16 (pak, 2));
   }

   else
      fprintf (logfile_fp, "\n                int16 pak_type = 0x%04x; // Unknown pak!", pak_type);

   // do a line break
   fprintf (logfile_fp, "\n");
   return;
}


static void WriteLineHeader (void)
{
   // this function prepares a log line header with the current date and time. It is shared by
   // most of the logging facilities.

   // THE T4C LOG STRING FORMAT IS \nSTRING (NEWLINE BEFORE DATA). This function converts
   // the standard string format STRING\n to T4C's format automatically. You do *NOT* have to
   // format your strings in the T4C format when calling these logging functions. Just do it
   // the usual way.

   SAFE_strncpy (log_string_header, "\n(T4C Guard)", sizeof (log_string_header)); // identifier
   log_string_header[12] = ',';
   _strdate_s (&log_string_header[13], sizeof (log_string_header) - 13 - 1); // "mm/dd/yy"
   log_string_header[21] = ',';
   _strtime_s (&log_string_header[22], sizeof (log_string_header) - 22 - 1); // "hh:mm:ss"
   log_string_header[30] = ',';
   log_string_header[31] = 0; // terminate the string

   return; // finished
}
