// launcher.c

#define DEFINE_GLOBALS
#include "rsrc-client.h"
#include "client.h"


// prototypes of functions used in this module
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpCmdLine, int nCmdShow);
static bool CheckDataFromServer (void);
static void SendPakToServer (pak_t *pak);
static void Config_Load (void);
static void Config_Save (void);


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpCmdLine, int nCmdShow)
{
   // program entrypoint

   STARTUPINFO T4CClient_si;
   char module[256];
   char cmdline[256];
   char *plogin;
   char *ppassword;
   char *pserver_ip;
   char *pserverport;
   int i;
   unsigned long length;
   unsigned short listenport;

   // save the application instance
   hAppInstance = hInstance;

   // do some init stuff
   srand ((unsigned int) time (NULL));
   memset (&T4CExecutable_pi, 0, sizeof (T4CExecutable_pi));
   memset (&config, 0, sizeof (config));
   our_socket = INVALID_SOCKET;

   // reset MOTD buffer
   messagetext[0] = 0;
   messagetext_modified = false;

   // find module and T4C Server path names
   GetModuleFileName (NULL, module, sizeof (module));
   Util_GetDirectoryPath (module, app_path);

   // read configuration data
   Config_Load ();

   // collect arguments from the command line
   SAFE_strncpy (cmdline, lpCmdLine, sizeof (cmdline));

   want_multiaccount = /*IS_SINGLEARG_PRESENT ("-multiaccount");*/ true; // TRUE so far.

   want_nowebpatch = IS_SINGLEARG_PRESENT ("-nowebpatch");
   log_level = IS_SINGLEARG_PRESENT ("-log");
   if (log_level > 0)
   {
      log_level = 2;
      if (unlink (LOG_MAIN) != 0)
         ; // make profiler happy
   }

   plogin = stristr (cmdline, "-uid=");
   ppassword = stristr (cmdline, "-lt=");
   pserver_ip = stristr (cmdline, "-ipserv=");
   pserverport = stristr (cmdline, "-port=");

   // parse the command line
   i = 0;
   while (cmdline[i] != 0)
   {
      // treat quoted strings as single words
      if (cmdline[i] == '"')
      {
         i++;
         while ((cmdline[i] != '"') && (cmdline[i] != 0))
            i++;
      }

      if ((cmdline[i] == ' ') || (cmdline[i] == '\t'))
         cmdline[i] = 0; // break the line in an array of arguments

      i++;
   }

   // if we were specified a login, take it in account
   if (plogin != NULL)
   {
      // copy login, skipping any leading quote if necessary
      SAFE_strncpy (last_login, (plogin[5] == '"' ? &plogin[6] : &plogin[5]), sizeof (last_login));
      length = strlen (last_login);
      if (last_login[length - 1] == '"')
         last_login[length - 1] = 0; // remove any trailing quote if necessary
   }

   // if we were specified a password, take it in account
   if (ppassword != NULL)
   {
      // copy password, skipping any leading quote if necessary
      SAFE_strncpy (password, (ppassword[4] == '"' ? &ppassword[5] : &ppassword[4]), sizeof (password));
      length = strlen (password);
      if (password[length - 1] == '"')
         password[length - 1] = 0; // remove any trailing quote if necessary
   }

   // if we were specified a server address, take it in account
   if (pserver_ip != NULL)
   {
      // copy server address, skipping any leading quote if necessary
      SAFE_strncpy (server_ip, (pserver_ip[8] == '"' ? &pserver_ip[9] : &pserver_ip[8]), sizeof (server_ip));
      length = strlen (server_ip);
      if (server_ip[length - 1] == '"')
         server_ip[length - 1] = 0; // remove any trailing quote if necessary
   }
   else
      SAFE_strncpy (server_ip, config.server_ip, sizeof (server_ip));

   // if we were specified a server port, take it in account
   if (pserverport != NULL)
   {
      // copy server port, skipping any leading quote if necessary
      SAFE_strncpy (server_port, (pserverport[6] == '"' ? &pserverport[7] : &pserverport[6]), sizeof (server_port));
      length = strlen (server_port);
      if (server_port[length - 1] == '"')
         server_port[length - 1] = 0; // remove any trailing quote if necessary
   }
   else
      SAFE_strncpy (server_port, config.server_port, sizeof (server_port));

   Network_Init (); // initialize the network layer

   listenport = (2048 + rand ()) % 32767; // generate a random port in the non-reserved range

   // create our socket and bind it to a listening port
   if ((our_socket = SenderReceiver_New (listenport)) == INVALID_SOCKET)
   {
      MessageBox_printf (MB_OK, "Erreur", "Impossible d'assigner le socket du service: %s\n", Util_GetLastNetworkError ());
      goto cleanup; // on error, quit the program
   }

   // fire up the launcher dialog box and let it run until we press the PLAY button
   if (stristr (cmdline, "-play") == NULL)
   {
      if (!DialogBox (hAppInstance, (char *) DIALOG_MAIN, NULL, MainDialogProc))
         goto cleanup; // we cancelled the dialog box
   }

   // do we want to webpatch ?
   if (!want_nowebpatch)
   {
      // TODO: webpatch code goes here
   }

   // build the argument string and start the T4C client
   memset (&T4CClient_si, 0, sizeof (T4CClient_si));
   T4CClient_si.cb = sizeof (T4CClient_si);
   SAFE_snprintf (module, sizeof (module), "%s\\t4c.exe", app_path);
   SAFE_snprintf (cmdline, sizeof (cmdline), "t4c.exe -uid=\"%s\" -lt=\"%s\" -ipserv=%s:%s", last_login, password, server_ip, server_port);
   if (!CreateProcess (module, cmdline, NULL, NULL, false, 0, NULL, app_path, &T4CClient_si, &T4CExecutable_pi))
   {
      MessageBox_printf (MB_OK, "Erreur", "Echec au lancement de t4c.exe: %s.", Util_GetLastError ());
      goto cleanup; // on error, exit the program
   }

cleanup:
   // finished processing, cleanup
   SenderReceiver_Destroy (&our_socket);
   Network_Shutdown ();

   // write configuration data
   Config_Save ();

   // free the CC macros array
   SAFE_free (&config.channelshortcuts);
   config.channelshortcut_count = 0;

   // free the CC colors array
   SAFE_free (&config.channelcolors);
   config.channelcolor_count = 0;

   // free the logins array
   SAFE_free (&config.accounts);
   config.account_count = 0;

   return (0); // return to Windows
}


void Loop_MOTD (void *thread_parms)
{
   // this thread contains the program's main transmit/receive loop that receives the MOTD

   pak_t *new_pak;
   float messagetext_check_time;
   int messagetext_tries;
   unsigned long host_address;

   messagetext_check_time = 0;
   messagetext_tries = 0;

   // get the server IP
   SAFE_strncpy (messagetext, "Localisation du serveur...", sizeof (messagetext));
   messagetext_modified = true;
   host_address = Util_LookupAddress (server_ip);
   if (host_address == INADDR_NONE)
   {
      MessageBox_printf (MB_OK, "Erreur", "Prophétie ne trouve pas l'adresse du serveur de jeu: %s\n", Util_GetLastNetworkError ());
      return;
   }

   // build the fake server service parameters
   SAFE_strncpy (messagetext, "Contact du serveur...", sizeof (messagetext));
   messagetext_modified = true;
   SAFE_snprintf (service_server.ip, 16, "%d.%d.%d.%d", host_address & 0xFF, (host_address >> 8) & 0xFF, (host_address >> 16) & 0xFF, (host_address >> 24) & 0xFF);
   service_server.port = atoi (server_port);

   // loop endlessly
   for (;;)
   {
      // see what time it is
      current_time = ProcessTime ();

      // is it time to check for the MOTD ?
      if (messagetext_check_time < current_time)
      {
         // have we requested it three times already ?
         if (messagetext_tries == 3)
         {
            SAFE_strncpy (messagetext, "Ce serveur semble être hors ligne.", sizeof (messagetext));
            break; // terminate the thread
         }

         // prepare a new "MOTD" pak and send it
         new_pak = Pak_New (0, PAKFLAG_CLIENT_PAK | PAKFLAG_CRYPTO_VIRCOM);
         Pak_AddInt16 (new_pak, PAK_CLIENT_MessageOfTheDay);
         SendPakToServer (new_pak); // send it to the server
         Pak_Destroy (new_pak);

         messagetext_check_time = current_time + 3.0f; // next check in 3.0 seconds
         messagetext_tries++; // we have requested this once more
      }

      // check and process datagrams that arrive from the server
      if (CheckDataFromServer ())
         break; // as soon as we get something, terminate the thread

      // now send what we have to send over the network
      Network_SendAll ();

      Sleep (1); // allow context switching if necessary (take our time...)
   }

   return; // finished
}


static bool CheckDataFromServer (void)
{
   // this function polls the loopback to check if data from the client has arrived, and if so,
   // encrypts it and transmits it to the server. It then does the same in the opposite way.

   datagram_t datagram;
   pak_t *pak;

   // wait for an incoming datagram from the network on the crypted port
   if (!Network_ReceiveAtSocket (&datagram, our_socket))
      return (false); // if no datagram, refresh the socket

   // we got a datagram
   Log_Text (LOG_MAIN, "==============================================================================");
   Log_Datagram (LOG_MAIN, &datagram, "Server to client:");

   // we got a packet, allocate space for it
   pak = Pak_New (0, PAKFLAG_SERVER_PAK | PAKFLAG_CRYPTO_VIRCOM);

   // remove Vircom's encryption and get the data
   if (!BuildPakFromUDPDatagrams (&datagram, PAKCRYPTO_VERSION_160, pak))
   {
      Pak_Destroy (pak); // skip this packet if we're told to
      return (false); // this is a fragmented pak and it's not fully arrived yet
   }

   // was this pak carrying an acknowledgment request ? if so, build the ack requests
   if (pak->flags & PAKFLAG_ACKREQUESTED)
      BuildAckReplyDatagramsFromPak (pak, PAKCRYPTO_VERSION_160, our_socket, &service_server);

   // given the type of pak it is, process the data
   if (Pak_ReadInt16 (pak, 0) == PAK_SERVER_MessageOfTheDay)
   {
      Pak_ReadString16 (pak, 2, messagetext, sizeof (messagetext)); // read the MOTD
      messagetext_modified = true; // and notify people that we've received it
   }

   Pak_Destroy (pak); // enough pak processing
   return (true); // stop listening for paks
}


static void SendPakToServer (pak_t *pak)
{
   // this function fakes the specified client sending a pre-built pak to the server

   // rebuild genuine-looking UDP packets from this pak into a new datagram list
   if (pak->flags & PAKFLAG_ACKREQUESTED)
      pak->flags = PAKFLAG_CLIENT_PAK | PAKFLAG_NEW_PAK | PAKFLAG_ACKREQUESTED;
   else
      pak->flags = PAKFLAG_CLIENT_PAK | PAKFLAG_NEW_PAK;

   // log these paks too
   Log_Pak (LOG_MAIN, pak, "Client to server:");
   Log_Text (LOG_MAIN, "==============================================================================");

   BuildUDPDatagramsFromPak (pak, PAKCRYPTO_VERSION_160, our_socket, &service_server);

   return; // finished
}


static void Config_Load (void)
{
   // this function opens and parses the INI configuration file for the program

   void *inidata;
   char filename[MAX_STRING8];
   char key[MAX_STRING8];
   char temp_string1[MAX_STRING8];
   char temp_string2[MAX_STRING8];
   unsigned long color;
   int index;
   int length;

   // open the INI file
   SAFE_snprintf (filename, sizeof (filename), "%s\\launcher.ini", app_path);
   inidata = INIFile_LoadINIFile (filename);

   // [server]
   SAFE_snprintf (config.server_name, sizeof (config.server_name), INIFile_ReadEntryAsString (inidata, "server", "name", "<server name>"));
   SAFE_snprintf (config.server_url, sizeof (config.server_url), INIFile_ReadEntryAsString (inidata, "server", "url", "http://www.the4thcoming.com"));
   SAFE_snprintf (config.server_ip, sizeof (config.server_ip), INIFile_ReadEntryAsString (inidata, "server", "ip", "127.0.0.1"));
   SAFE_snprintf (config.server_port, sizeof (config.server_port), INIFile_ReadEntryAsString (inidata, "server", "port", "11677"));

   // [accounts]
   SAFE_snprintf (config.accounts_lastlogin, sizeof (config.accounts_lastlogin), INIFile_ReadEntryAsString (inidata, "accounts", "last login", ""));
   config.account_count = 0; // read logins
   for (;;)
   {
      // try to read a new login/password pair
      SAFE_snprintf (key, sizeof (key), "username %d", config.account_count);
      SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "accounts", key, ""));
      SAFE_snprintf (key, sizeof (key), "password %d", config.account_count);
      SAFE_snprintf (temp_string2, sizeof (temp_string2), INIFile_ReadEntryAsString (inidata, "accounts", key, ""));

      // did we get NO data ?
      if (temp_string1[0] == 0)
         break; // if so, stop it

      // we have a new login/password pair, reallocate space to hold it
      config.accounts = (account_t *) SAFE_realloc (config.accounts, config.account_count, config.account_count + 1, sizeof (account_t), false);

      // copy login and deXORed password in place
      SAFE_strncpy (config.accounts[config.account_count].username, temp_string1, sizeof (config.accounts[config.account_count].username));
      length = strlen (temp_string2); // deXOR password
      for (index = 0; index < length; index += 2)
      {
         if (sscanf_s (&temp_string2[index], "%02X", &config.accounts[config.account_count].password[index / 2], 1) != 1)
            break; // read byte after byte (2 ASCII chars) unless error
         config.accounts[config.account_count].password[index / 2] ^= 0x69; // XOR with this
      }
      config.accounts[config.account_count].password[length / 2] = 0; // terminate the string

      config.account_count++; // remember we know now one login/password pair more
   }

   // [options]
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "options", "language", "english"));
   if (strcmp (temp_string1, "french") == 0)
      config.options_language = LANGUAGE_FRENCH;
   else
      config.options_language = LANGUAGE_ENGLISH;

   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "options", "screen resolution", "1024x768"));
   if (strcmp (temp_string1, "640x480") == 0)
      config.options_screenresolution = SCREENRESOLUTION_640X480;
   else if (strcmp (temp_string1, "800x600") == 0)
      config.options_screenresolution = SCREENRESOLUTION_800X600;
   else
      config.options_screenresolution = SCREENRESOLUTION_1024X768;
   config.options_fullscreen = (INIFile_ReadEntryAsBool (inidata, "options", "fullscreen", false) > 0);
   config.options_keepaspectratio = (INIFile_ReadEntryAsBool (inidata, "options", "keep aspect ratio", false) > 0);
   config.options_waitforvsync = (INIFile_ReadEntryAsBool (inidata, "options", "wait for vertical sync", true) > 0);
   config.options_debuglevel = INIFile_ReadEntryAsLong (inidata, "options", "debug level", 0);

   // [shortcut keys]
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "inventory", "I"));
   config.shortcutkeys_inventory = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "character", "S"));
   config.shortcutkeys_character = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "chat window", "L"));
   config.shortcutkeys_chatwindow = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "group", "G"));
   config.shortcutkeys_group = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "spellbook", "P"));
   config.shortcutkeys_spellbook = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "macros", "M"));
   config.shortcutkeys_macros = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "exchange", "T"));
   config.shortcutkeys_exchange = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "options", "O"));
   config.shortcutkeys_options = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "screenshot", "H"));
   config.shortcutkeys_screenshot = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "big chat window", "A"));
   config.shortcutkeys_bigchatwindow = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "area map", "W"));
   config.shortcutkeys_areamap = temp_string1[0];
   SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "shortcut keys", "attack mode", "C"));
   config.shortcutkeys_attackmode = temp_string1[0];

   // [channel colors]
   config.channelcolor_count = 0; // read channel colors
   for (;;)
   {
      // try to read a new color
      SAFE_snprintf (key, sizeof (key), "color %d", config.channelcolor_count);
      SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "channel colors", key, ""));

      // did we get NO data ?
      if (sscanf_s (temp_string1, "%x", &color) != 1)
         break; // if so, stop it

      // we have a new color, reallocate space to hold it
      config.channelcolors = (unsigned long *) SAFE_realloc (config.channelcolors, config.channelcolor_count, config.channelcolor_count + 1, sizeof (unsigned long), false);
      config.channelcolors[config.channelcolor_count] = color; // save it
      config.channelcolor_count++; // remember we know now one color more
   }

   // [channel shortcuts]
   config.channelshortcut_count = 0;
   for (;;)
   {
      SAFE_snprintf (key, sizeof (key), "channel %d", config.channelshortcut_count);
      SAFE_snprintf (temp_string1, sizeof (temp_string1), INIFile_ReadEntryAsString (inidata, "channel shortcuts", key, ""));
      SAFE_snprintf (key, sizeof (key), "shortcut %d", config.channelshortcut_count);
      SAFE_snprintf (temp_string2, sizeof (temp_string2), INIFile_ReadEntryAsString (inidata, "channel shortcuts", key, ""));

      // did we get NO data ?
      if (temp_string1[0] == 0)
         break; // if so, stop it

      // see if we already know this channel
      for (index = 0; index < config.channelshortcut_count; index++)
         if (stricmp (config.channelshortcuts[index].channel, temp_string1) == 0)
            break; // break as soon as we find it

      // if not, see if we already know this shortcut
      if (index == config.channelshortcut_count)
         for (index = 0; index < config.channelshortcut_count; index++)
            if (stricmp (config.channelshortcuts[index].shortcut, temp_string2) == 0)
               break; // break as soon as we find it

      // if still not, we have a new channel/shortcut pair, reallocate space to hold it
      if (index == config.channelshortcut_count)
      {
         config.channelshortcuts = (channelshortcut_t *) SAFE_realloc (config.channelshortcuts, config.channelshortcut_count, config.channelshortcut_count + 1, sizeof (channelshortcut_t), false);
         config.channelshortcut_count++; // remember we know now one channel/shortcut pair more
      }

      // copy channel and shortcut in place
      SAFE_strncpy (config.channelshortcuts[index].channel, temp_string1, sizeof (config.channelshortcuts[index].channel));
      SAFE_strncpy (config.channelshortcuts[index].shortcut, temp_string2, sizeof (config.channelshortcuts[index].shortcut));
   }

   // close the INI file
   INIFile_FreeINIFile (inidata);
   return; // finished loading config
}


static void Config_Save (void)
{
   // this function saves the software configuration into an INI file

   void *inidata;
   char filename[MAX_STRING8];
   char encrypted_password[MAX_STRING8];
   char key[MAX_STRING8];
   char temp_string[MAX_STRING8];
   int index;
   int index2;
   int length;

   // create a config file with the default values and a dummy plugin
   inidata = INIFile_NewINIFile ();

   // [server]
   INIFile_WriteEntryAsString (inidata, "server", "name", config.server_name);
   INIFile_WriteEntryAsString (inidata, "server", "url", config.server_url);
   INIFile_WriteEntryAsString (inidata, "server", "ip", config.server_ip);
   INIFile_WriteEntryAsString (inidata, "server", "port", config.server_port);

   // [accounts]
   INIFile_WriteEntryAsString (inidata, "accounts", "last login", config.accounts_lastlogin);
   for (index = 0; index < config.account_count; index++)
   {
      // build the login/password pair and write login
      SAFE_snprintf (key, sizeof (key), "username %d", index);
      INIFile_WriteEntryAsString (inidata, "accounts", key, config.accounts[index].username);

      // enXOR password
      encrypted_password[0] = 0;
      length = strlen (config.accounts[index].password);
      for (index2 = 0; index2 < length; index2++)
         SAFE_strncatf (encrypted_password, sizeof (encrypted_password), "%02X", config.accounts[index].password[index2] ^ 0x69); // XOR with this

      // write encrypted password
      SAFE_snprintf (key, sizeof (key), "password %d", index);
      INIFile_WriteEntryAsString (inidata, "accounts", key, encrypted_password);
   }

   // [options]
   if (config.options_language == LANGUAGE_FRENCH)
      INIFile_WriteEntryAsString (inidata, "options", "language", "french");
   else
      INIFile_WriteEntryAsString (inidata, "options", "language", "english");
   if (config.options_screenresolution == SCREENRESOLUTION_640X480)
      INIFile_WriteEntryAsString (inidata, "options", "screen resolution", "640x480");
   else if (config.options_screenresolution == SCREENRESOLUTION_800X600)
      INIFile_WriteEntryAsString (inidata, "options", "screen resolution", "800x600");
   else
      INIFile_WriteEntryAsString (inidata, "options", "screen resolution", "1024x768");
   INIFile_WriteEntryAsBool (inidata, "options", "fullscreen", config.options_fullscreen);
   INIFile_WriteEntryAsBool (inidata, "options", "keep aspect ratio", config.options_keepaspectratio);
   INIFile_WriteEntryAsBool (inidata, "options", "wait for vertical sync", config.options_waitforvsync);
   INIFile_WriteEntryAsLong (inidata, "options", "debug level", config.options_debuglevel);

   // [shortcut keys]
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_inventory);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "inventory", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_character);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "character", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_chatwindow);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "chat window", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_group);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "group", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_spellbook);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "spellbook", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_macros);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "macros", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_exchange);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "exchange", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_options);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "options", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_screenshot);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "screenshot", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_bigchatwindow);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "big chat window", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_areamap);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "area map", temp_string);
   SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_attackmode);
   INIFile_WriteEntryAsString (inidata, "shortcut keys", "attack mode", temp_string);

   // [channel colors]
   for (index = 0; index < config.channelcolor_count; index++)
   {
      SAFE_snprintf (key, sizeof (key), "color %d", index); // build key and dump value
      SAFE_snprintf (temp_string, sizeof (temp_string), "%06x", config.channelcolors[index]); // build key and dump value
      INIFile_WriteEntryAsString (inidata, "channel colors", key, temp_string);
   }

   // [channel shortcuts]
   for (index = 0; index < config.channelshortcut_count; index++)
   {
      // build the channel/shortcut pair and write it
      SAFE_snprintf (key, sizeof (key), "channel %d", index);
      INIFile_WriteEntryAsString (inidata, "channel shortcuts", key, config.channelshortcuts[index].channel);
      SAFE_snprintf (key, sizeof (key), "shortcut %d", index);
      INIFile_WriteEntryAsString (inidata, "channel shortcuts", key, config.channelshortcuts[index].shortcut);
   }

   // don't save the hidden options

   // open the INI file
   SAFE_snprintf (filename, sizeof (filename), "%s\\launcher.ini", app_path);
   INIFile_SaveINIFile (filename, inidata);

   return; // finished, configuration data is written
}
