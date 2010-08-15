// config.c

#include "common.h"


void Config_Init (void)
{
   // this function opens and parses the INI configuration file for the program

   void *inifile;
   char key[MAX_STRING8];
   char value[MAX_STRING8];
   int plugin_index;
   config_t *config;

   config = &server.config; // quick access to server config

   // open the INI file
   SAFE_snprintf (config->inifile_pathname, sizeof (config->inifile_pathname), "%s\\t4c guard.ini", server.app_path);
   inifile = INIFile_LoadINIFile (config->inifile_pathname);

   // file found ?
   if (inifile != NULL)
   {
      // [general]
      config->general.listen_port = (unsigned short) INIFile_ReadEntryAsLong (inifile, "general", "listen port", 443);
      config->general.max_plugins = INIFile_ReadEntryAsLong (inifile, "general", "max plugins", 100);
      config->general.max_clients = INIFile_ReadEntryAsLong (inifile, "general", "max clients", 512);
      config->general.max_units = INIFile_ReadEntryAsLong (inifile, "general", "max units", 100000);
      config->general.min_frametime = 1.0f / (float) INIFile_ReadEntryAsLong (inifile, "general", "framerate limit", 100);
      config->general.sleep_skips = INIFile_ReadEntryAsLong (inifile, "general", "sleep skips", 10);
      config->general.client_timeout = INIFile_ReadEntryAsLong (inifile, "general", "client timeout", 30);
      config->general.want_encryption = (INIFile_ReadEntryAsBool (inifile, "general", "encryption required", true) > 0);
      config->general.wanted_version = INIFile_ReadEntryAsLong (inifile, "general", "version required", 0);
      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "general", "update script", ""));
      Util_MakeAbsolutePathname (value, config->general.updatescript_filename, sizeof (config->general.updatescript_filename));
      SAFE_snprintf (config->general.webserver_url, sizeof (config->general.webserver_url), INIFile_ReadEntryAsString (inifile, "general", "webserver url", "http://www.the4thcoming.com"));
      SAFE_snprintf (config->general.welcome_message, sizeof (config->general.welcome_message), INIFile_ReadEntryAsString (inifile, "general", "welcome message", "Welcome, adventurer, in the world of Althea."));

      // [server]
      SAFE_snprintf (config->t4cserver.address, sizeof (config->t4cserver.address), INIFile_ReadEntryAsString (inifile, "server", "address", "127.0.0.1"));
      config->t4cserver.port = (unsigned short) INIFile_ReadEntryAsLong (inifile, "server", "port", 11677);
      SAFE_snprintf (config->t4cserver.start_command, sizeof (config->t4cserver.start_command), INIFile_ReadEntryAsString (inifile, "server", "start command", "T4C Server.exe --spawn_service"));
      config->t4cserver.shutdown_when_closing = (INIFile_ReadEntryAsBool (inifile, "server", "shutdown when closing", true) > 0);
      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "server", "MOTD file", "motd.txt"));
      Util_MakeAbsolutePathname (value, config->t4cserver.motd_pathname, sizeof (config->t4cserver.motd_pathname));

      // [database]
      SAFE_snprintf (config->db.dsn_name, sizeof (config->db.dsn_name), INIFile_ReadEntryAsString (inifile, "database", "data source name", "T4C Server"));
      SAFE_snprintf (config->db.username, sizeof (config->db.username), INIFile_ReadEntryAsString (inifile, "database", "username", "Account"));
      SAFE_snprintf (config->db.password, sizeof (config->db.password), INIFile_ReadEntryAsString (inifile, "database", "password", "Password"));

      // [webpatch]
      SAFE_snprintf (config->webpatch.url, sizeof (config->webpatch.url), INIFile_ReadEntryAsString (inifile, "webpatch", "patch file url", ""));
      SAFE_snprintf (config->webpatch.login, sizeof (config->webpatch.login), INIFile_ReadEntryAsString (inifile, "webpatch", "login", ""));
      SAFE_snprintf (config->webpatch.password, sizeof (config->webpatch.password), INIFile_ReadEntryAsString (inifile, "webpatch", "password", ""));

      // [admin bot]
      SAFE_snprintf (config->adminbot.channel_name, sizeof (config->adminbot.channel_name), INIFile_ReadEntryAsString (inifile, "admin bot", "channel_name", "Main"));
      SAFE_snprintf (config->adminbot.channel_password, sizeof (config->adminbot.channel_password), INIFile_ReadEntryAsString (inifile, "admin bot", "channel_password", ""));
      SAFE_snprintf (config->adminbot.playername, sizeof (config->adminbot.playername), INIFile_ReadEntryAsString (inifile, "admin bot", "player name", T4CGUARD_NAME));

      // [no-ip]
      config->noip.enabled = (INIFile_ReadEntryAsBool (inifile, "no-ip", "enable updates", false) > 0);
      SAFE_snprintf (config->noip.hostname, sizeof (config->noip.hostname), INIFile_ReadEntryAsString (inifile, "no-ip", "hostname", "myserver.servegame.com"));
      SAFE_snprintf (config->noip.login, sizeof (config->noip.login), INIFile_ReadEntryAsString (inifile, "no-ip", "login", "myemail@hotmail.com"));
      SAFE_snprintf (config->noip.password, sizeof (config->noip.password), INIFile_ReadEntryAsString (inifile, "no-ip", "password", "mypassword"));
      config->noip.check_interval = (float) INIFile_ReadEntryAsLong (inifile, "no-ip", "check interval in seconds", 300);

      // [rfb]
      config->rfb.enabled = (INIFile_ReadEntryAsBool (inifile, "rfb", "enabled", true) > 0);
      SAFE_snprintf (config->rfb.password, sizeof (config->rfb.password), INIFile_ReadEntryAsString (inifile, "rfb", "password", "mypassword"));

      // [log]
      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "log", "main log file", "t4c guard.log"));
      Util_MakeAbsolutePathname (value, config->log.mainlog.filename, sizeof (config->log.mainlog.filename));
      config->log.mainlog.level = (unsigned char) INIFile_ReadEntryAsLong (inifile, "log", "main log level", 0);
      config->log.mainlog.reset = (INIFile_ReadEntryAsBool (inifile, "log", "main log reset", false) > 0);
      if (config->log.mainlog.reset && unlink (config->log.mainlog.filename)); // care of the return value to make profiler happy

      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "log", "death log file", "death.log"));
      Util_MakeAbsolutePathname (value, config->log.deathlog.filename, sizeof (config->log.deathlog.filename));
      config->log.deathlog.level = (unsigned char) INIFile_ReadEntryAsLong (inifile, "log", "death log level", 0);
      config->log.deathlog.reset = (INIFile_ReadEntryAsBool (inifile, "log", "death log reset", false) > 0);
      if (config->log.deathlog.reset && unlink (config->log.deathlog.filename)); // care of the return value to make profiler happy

      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "log", "gameop log file", "gameop.log"));
      Util_MakeAbsolutePathname (value, config->log.gameoplog.filename, sizeof (config->log.gameoplog.filename));
      config->log.gameoplog.level = (unsigned char) INIFile_ReadEntryAsLong (inifile, "log", "gameop log level", 0);
      config->log.gameoplog.reset = (INIFile_ReadEntryAsBool (inifile, "log", "gameop log reset", false) > 0);
      if (config->log.gameoplog.reset && unlink (config->log.gameoplog.filename)); // care of the return value to make profiler happy

      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "log", "items log file", "items.log"));
      Util_MakeAbsolutePathname (value, config->log.itemslog.filename, sizeof (config->log.itemslog.filename));
      config->log.itemslog.level = (unsigned char) INIFile_ReadEntryAsLong (inifile, "log", "items log level", 0);
      config->log.itemslog.reset = (INIFile_ReadEntryAsBool (inifile, "log", "items log reset", false) > 0);
      if (config->log.itemslog.reset && unlink (config->log.itemslog.filename)); // care of the return value to make profiler happy

      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "log", "npcs log file", "npcs.log"));
      Util_MakeAbsolutePathname (value, config->log.npcslog.filename, sizeof (config->log.npcslog.filename));
      config->log.npcslog.level = (unsigned char) INIFile_ReadEntryAsLong (inifile, "log", "npcs log level", 0);
      config->log.npcslog.reset = (INIFile_ReadEntryAsBool (inifile, "log", "npcs log reset", false) > 0);
      if (config->log.npcslog.reset && unlink (config->log.npcslog.filename)); // care of the return value to make profiler happy

      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "log", "pages log file", "pages.log"));
      Util_MakeAbsolutePathname (value, config->log.pageslog.filename, sizeof (config->log.pageslog.filename));
      config->log.pageslog.level = (unsigned char) INIFile_ReadEntryAsLong (inifile, "log", "pages log level", 0);
      config->log.pageslog.reset = (INIFile_ReadEntryAsBool (inifile, "log", "pages log reset", false) > 0);
      if (config->log.pageslog.reset && unlink (config->log.pageslog.filename)); // care of the return value to make profiler happy

      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "log", "shouts log file", "shouts.log"));
      Util_MakeAbsolutePathname (value, config->log.shoutslog.filename, sizeof (config->log.shoutslog.filename));
      config->log.shoutslog.level = (unsigned char) INIFile_ReadEntryAsLong (inifile, "log", "shouts log level", 0);
      config->log.shoutslog.reset = (INIFile_ReadEntryAsBool (inifile, "log", "shouts log reset", false) > 0);
      if (config->log.shoutslog.reset && unlink (config->log.shoutslog.filename)); // care of the return value to make profiler happy

      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "log", "talk log file", "onlinetext.log"));
      Util_MakeAbsolutePathname (value, config->log.talklog.filename, sizeof (config->log.talklog.filename));
      config->log.talklog.level = (unsigned char) INIFile_ReadEntryAsLong (inifile, "log", "talk log level", 0);
      config->log.talklog.reset = (INIFile_ReadEntryAsBool (inifile, "log", "talk log reset", false) > 0);
      if (config->log.talklog.reset && unlink (config->log.talklog.filename)); // care of the return value to make profiler happy

      // [reboot]
      config->reboot.enabled = (INIFile_ReadEntryAsBool (inifile, "reboot", "enable daily reboot", true) > 0);
      SAFE_snprintf (config->reboot.at_time, sizeof (config->reboot.at_time), INIFile_ReadEntryAsString (inifile, "reboot", "reboot time", "23:55"));
      config->reboot.timeout = INIFile_ReadEntryAsLong (inifile, "reboot", "reboot delay", 5);

      // [backup]
      config->backup.enabled = (INIFile_ReadEntryAsBool (inifile, "backup", "enable backup script", true) > 0);
      SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "backup", "script name", "backup.bat"));
      Util_MakeAbsolutePathname (value, config->backup.script_filename, sizeof (config->backup.script_filename));

      // [plugins]
      config->pluglist.count = 0;
      for (plugin_index = 0; plugin_index < 999; plugin_index++)
      {
         SAFE_snprintf (key, sizeof (key), "plugin%02d", plugin_index + 1);
         SAFE_snprintf (value, sizeof (value), INIFile_ReadEntryAsString (inifile, "plugins", key, ""));
         if (value[0] != 0)
         {
            // reallocate the list of plugins to be loaded at boot time to hold one plugin path more
            config->pluglist.pluginpaths = (config_plugpath_t *) SAFE_realloc (config->pluglist.pluginpaths, config->pluglist.count, config->pluglist.count + 1, sizeof (config_plugpath_t), true);
            strcpy (config->pluglist.pluginpaths[config->pluglist.count].path, value); // copy plugin path
            config->pluglist.count++; // this list has now one plugin more
         }
      }
   }

   // else we need to create a config file
   else
   {
      // create a config file with the default values and a dummy plugin
      inifile = INIFile_NewINIFile ();

      // [general]
      INIFile_WriteEntryAsLong (inifile, "general", "listen port", config->general.listen_port);
      INIFile_WriteEntryAsLong (inifile, "general", "max plugins", config->general.max_plugins);
      INIFile_WriteEntryAsLong (inifile, "general", "max clients", config->general.max_clients);
      INIFile_WriteEntryAsLong (inifile, "general", "max units", config->general.max_units);
      INIFile_WriteEntryAsLong (inifile, "general", "framerate limit", (unsigned long) (1.0f / config->general.min_frametime));
      INIFile_WriteEntryAsLong (inifile, "general", "sleep skips", config->general.sleep_skips);
      INIFile_WriteEntryAsLong (inifile, "general", "client timeout", config->general.client_timeout);
      INIFile_WriteEntryAsBool (inifile, "general", "encryption required", config->general.want_encryption);
      INIFile_WriteEntryAsLong (inifile, "general", "version required", config->general.wanted_version);
      INIFile_WriteEntryAsString (inifile, "general", "update script", config->general.updatescript_filename);
      INIFile_WriteEntryAsString (inifile, "general", "webserver url", config->general.webserver_url);
      INIFile_WriteEntryAsString (inifile, "general", "welcome message", config->general.welcome_message);

      // [server]
      INIFile_WriteEntryAsString (inifile, "server", "address", config->t4cserver.address);
      INIFile_WriteEntryAsLong (inifile, "server", "port", config->t4cserver.port);
      INIFile_WriteEntryAsString (inifile, "server", "start command", config->t4cserver.start_command);
      INIFile_WriteEntryAsBool (inifile, "server", "shutdown when closing", config->t4cserver.shutdown_when_closing);
      INIFile_WriteEntryAsString (inifile, "server", "MOTD file", config->t4cserver.motd_pathname);

      // [database]
      INIFile_WriteEntryAsString (inifile, "database", "data source name", config->db.dsn_name);
      INIFile_WriteEntryAsString (inifile, "database", "username", config->db.username);
      INIFile_WriteEntryAsString (inifile, "database", "password", config->db.password);

      // [webpatch]
      INIFile_WriteEntryAsString (inifile, "webpatch", "patch file url", config->webpatch.url);
      INIFile_WriteEntryAsString (inifile, "webpatch", "login", config->webpatch.login);
      INIFile_WriteEntryAsString (inifile, "webpatch", "password", config->webpatch.password);

      // [general]
      INIFile_WriteEntryAsString (inifile, "admin bot", "channel", config->adminbot.channel_name);
      INIFile_WriteEntryAsString (inifile, "admin bot", "password", config->adminbot.channel_password);
      INIFile_WriteEntryAsString (inifile, "admin bot", "player name", config->adminbot.playername);

      // [no-ip]
      INIFile_WriteEntryAsBool (inifile, "no-ip", "enable updates", config->noip.enabled);
      INIFile_WriteEntryAsString (inifile, "no-ip", "hostname", config->noip.hostname);
      INIFile_WriteEntryAsString (inifile, "no-ip", "login", config->noip.login);
      INIFile_WriteEntryAsString (inifile, "no-ip", "password", config->noip.password);
      INIFile_WriteEntryAsLong (inifile, "no-ip", "check interval in seconds", (unsigned long) config->noip.check_interval);

      // [rfb]
      INIFile_WriteEntryAsBool (inifile, "rfb", "enabled", true);
      INIFile_WriteEntryAsString (inifile, "rfb", "password", config->rfb.password);

      // [log]
      INIFile_WriteEntryAsString (inifile, "log", "main log file", config->log.mainlog.filename);
      INIFile_WriteEntryAsLong (inifile, "log", "main log level", config->log.mainlog.level);
      INIFile_WriteEntryAsBool (inifile, "log", "main log reset", config->log.mainlog.reset);
      INIFile_WriteEntryAsString (inifile, "log", "death log file", config->log.deathlog.filename);
      INIFile_WriteEntryAsLong (inifile, "log", "death log level", config->log.deathlog.level);
      INIFile_WriteEntryAsBool (inifile, "log", "death log reset", config->log.deathlog.reset);
      INIFile_WriteEntryAsString (inifile, "log", "gameop log file", config->log.gameoplog.filename);
      INIFile_WriteEntryAsLong (inifile, "log", "gameop log level", config->log.gameoplog.level);
      INIFile_WriteEntryAsBool (inifile, "log", "gameop log reset", config->log.gameoplog.reset);
      INIFile_WriteEntryAsString (inifile, "log", "items log file", config->log.itemslog.filename);
      INIFile_WriteEntryAsLong (inifile, "log", "items log level", config->log.itemslog.level);
      INIFile_WriteEntryAsBool (inifile, "log", "items log reset", config->log.itemslog.reset);
      INIFile_WriteEntryAsString (inifile, "log", "npcs log file", config->log.npcslog.filename);
      INIFile_WriteEntryAsLong (inifile, "log", "npcs log level", config->log.npcslog.level);
      INIFile_WriteEntryAsBool (inifile, "log", "npcs log reset", config->log.npcslog.reset);
      INIFile_WriteEntryAsString (inifile, "log", "pages log file", config->log.pageslog.filename);
      INIFile_WriteEntryAsLong (inifile, "log", "pages log level", config->log.pageslog.level);
      INIFile_WriteEntryAsBool (inifile, "log", "pages log reset", config->log.pageslog.reset);
      INIFile_WriteEntryAsString (inifile, "log", "shouts log file", config->log.shoutslog.filename);
      INIFile_WriteEntryAsLong (inifile, "log", "shouts log level", config->log.shoutslog.level);
      INIFile_WriteEntryAsBool (inifile, "log", "shouts log reset", config->log.shoutslog.reset);
      INIFile_WriteEntryAsString (inifile, "log", "talk log file", config->log.talklog.filename);
      INIFile_WriteEntryAsLong (inifile, "log", "talk log level", config->log.talklog.level);
      INIFile_WriteEntryAsBool (inifile, "log", "talk log reset", config->log.talklog.reset);

      // [reboot]
      INIFile_WriteEntryAsBool (inifile, "reboot", "enable daily reboot", config->reboot.enabled);
      INIFile_WriteEntryAsString (inifile, "reboot", "reboot time", config->reboot.at_time);
      INIFile_WriteEntryAsLong (inifile, "reboot", "reboot delay", config->reboot.timeout);

      // [backup]
      INIFile_WriteEntryAsBool (inifile, "backup", "enable backup script", config->backup.enabled);
      INIFile_WriteEntryAsString (inifile, "backup", "script name", config->backup.script_filename);

      // [plugins]
      INIFile_WriteEntryAsString (inifile, "plugins", "plugin01", "relative/path/to/the/file.dll");
      INIFile_WriteEntryAsString (inifile, "plugins", "plugin02", "relative/path/to/the/file.dll");

      INIFile_SaveINIFile (config->inifile_pathname, inifile);
   }

   // close the INI file
   INIFile_FreeINIFile (inifile);

   Log_Text (LOG_MAIN, "InitConfig(): Configuration data read successfully.\n");
   return; // finished loading config
}


void Config_Shutdown (void)
{
   // this function frees all memory space that was allocated for the configuration data

   SAFE_free (&server.config.pluglist.pluginpaths);

   return; // finished, configuration data is freed
}
