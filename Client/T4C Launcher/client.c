// client.c

#include "common.h"


#define HASH_GRANULARITY (2 ^ 4) // hashtable interpolation value, must be 2^X


// local typedefs
typedef struct admin_command_s
{
   __int64 required_flag; // required gameop flag to run command
   char *prefix; // will be strnicmp'ed
   char *suffix; // will be stristr'ed
   bool must_grant; // if TRUE, concerned_flag will be granted ; if FALSE it'll be withdrawn
   __int64 granted_flag; // flag that will be either granted to or removed from target
} admin_command_t;


// global variables for internal use only
static hashtable_bucket_t hashtable[HASH_GRANULARITY];
static client_t *clients; // in-game clients (mallocated)
static int client_count; // current amount of client structures in the previous array
//static char motd_text[MAX_STRING16];
static admin_command_t admin_commands[] =
{
   { GOD_CAN_LOCKOUT_USER,   "lockout ", "", true, USER_LOCKED_OUT },
   { GOD_CAN_SQUELCH,        "squelch ", "", true, USER_CANNOT_TALK_WITH_NPCS_PLAYERS },
   { GOD_CAN_SQUELCH,        "unsquelch ", "", false, USER_CANNOT_TALK_WITH_NPCS_PLAYERS },
   { GOD_CAN_REMOVE_SHOUTS,  "remove ", "'s shouts", true, USER_CANNOT_SHOUT_IN_A_CHATTER_CHANNEL },
   { GOD_CAN_REMOVE_SHOUTS,  "restore ", "'s shouts", false, USER_CANNOT_SHOUT_IN_A_CHATTER_CHANNEL },
   { GOD_CAN_SEE_ACCOUNTS,   "remove ", " from user listing", true, USER_DOES_NOT_APPEAR_IN_USER_LISTING },
   { GOD_CAN_SEE_ACCOUNTS,   "restore ", " to user listing", false, USER_DOES_NOT_APPEAR_IN_USER_LISTING },
   { GOD_CAN_SLAY_USER,      "remove ", "'s right to pvp", true, USER_CANNOT_ENGAGE_IN_PVP_AGAINST_ANYONE },
   { GOD_CAN_SLAY_USER,      "restore ", "'s right to pvp", false, USER_CANNOT_ENGAGE_IN_PVP_AGAINST_ANYONE },
   { GOD_CAN_REMOVE_SHOUTS,  "remove ", "'s pages", true, USER_CANNOT_PAGE_ANOTHER_USER },
   { GOD_CAN_REMOVE_SHOUTS,  "restore ", "'s pages", false, USER_CANNOT_PAGE_ANOTHER_USER },
   { GOD_CAN_SLAY_USER,      "set full pvp for ", "", true, USER_CAN_ENGAGE_IN_PVP_AGAINST_ANYONE },
   { GOD_CAN_SLAY_USER,      "remove full pvp for ", "", false, USER_CAN_ENGAGE_IN_PVP_AGAINST_ANYONE },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_NO_CLIP to ", "", true, GOD_NO_CLIP },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_NO_CLIP from ", "", false, GOD_NO_CLIP },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_NO_MONSTERS to ", "", true, GOD_NO_MONSTERS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_NO_MONSTERS from ", "", false, GOD_NO_MONSTERS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_TELEPORT to ", "", true, GOD_CAN_TELEPORT },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_TELEPORT from ", "", false, GOD_CAN_TELEPORT },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_TELEPORT_USER to ", "", true, GOD_CAN_TELEPORT_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_TELEPORT_USER from ", "", false, GOD_CAN_TELEPORT_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_ZAP to ", "", true, GOD_CAN_ZAP },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_ZAP from ", "", false, GOD_CAN_ZAP },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_SQUELCH to ", "", true, GOD_CAN_SQUELCH },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_SQUELCH from ", "", false, GOD_CAN_SQUELCH },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_REMOVE_SHOUTS to ", "", true, GOD_CAN_REMOVE_SHOUTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_REMOVE_SHOUTS from ", "", false, GOD_CAN_REMOVE_SHOUTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_REMOVE_SHOUTS to ", "", true, GOD_CAN_REMOVE_SHOUTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_REMOVE_SHOUTS from ", "", false, GOD_CAN_REMOVE_SHOUTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_SUMMON_MONSTERS to ", "", true, GOD_CAN_SUMMON_MONSTERS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_SUMMON_MONSTERS from ", "", false, GOD_CAN_SUMMON_MONSTERS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_SUMMON_ITEMS to ", "", true, GOD_CAN_SUMMON_ITEMS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_SUMMON_ITEMS from ", "", false, GOD_CAN_SUMMON_ITEMS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_SET_USER_FLAG to ", "", true, GOD_CAN_SET_USER_FLAG },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_SET_USER_FLAG from ", "", false, GOD_CAN_SET_USER_FLAG },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER to ", "", true, GOD_CAN_EDIT_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER from ", "", false, GOD_CAN_EDIT_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_STAT to ", "", true, GOD_CAN_EDIT_USER_STAT },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_STAT from ", "", false, GOD_CAN_EDIT_USER_STAT },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_HP to ", "", true, GOD_CAN_EDIT_USER_HP },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_HP from ", "", false, GOD_CAN_EDIT_USER_HP },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_MANA_FAITH to ", "", true, GOD_CAN_EDIT_USER_MANA_FAITH },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_MANA_FAITH from ", "", false, GOD_CAN_EDIT_USER_MANA_FAITH },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_XP_LEVEL to ", "", true, GOD_CAN_EDIT_USER_XP_LEVEL },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_XP_LEVEL from ", "", false, GOD_CAN_EDIT_USER_XP_LEVEL },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_NAME to ", "", true, GOD_CAN_EDIT_USER_NAME },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_NAME from ", "", false, GOD_CAN_EDIT_USER_NAME },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_APPEARANCE_CORPSE to ", "", true, GOD_CAN_EDIT_USER_APPEARANCE_CORPSE },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_APPEARANCE_CORPSE from ", "", false, GOD_CAN_EDIT_USER_APPEARANCE_CORPSE },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_SPELLS to ", "", true, GOD_CAN_EDIT_USER_SPELLS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_SPELLS from ", "", false, GOD_CAN_EDIT_USER_SPELLS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_SKILLS to ", "", true, GOD_CAN_EDIT_USER_SKILLS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_SKILLS from ", "", false, GOD_CAN_EDIT_USER_SKILLS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EDIT_USER_BACKPACK to ", "", true, GOD_CAN_EDIT_USER_BACKPACK },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EDIT_USER_BACKPACK from ", "", false, GOD_CAN_EDIT_USER_BACKPACK },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_VIEW_USER to ", "", true, GOD_CAN_VIEW_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_VIEW_USER from ", "", false, GOD_CAN_VIEW_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_VIEW_USER_STAT to ", "", true, GOD_CAN_VIEW_USER_STAT },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_VIEW_USER_STAT from ", "", false, GOD_CAN_VIEW_USER_STAT },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_VIEW_USER_BACKPACK to ", "", true, GOD_CAN_VIEW_USER_BACKPACK },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_VIEW_USER_BACKPACK from ", "", false, GOD_CAN_VIEW_USER_BACKPACK },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_VIEW_USER_SPELLS to ", "", true, GOD_CAN_VIEW_USER_SPELLS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_VIEW_USER_SPELLS from ", "", false, GOD_CAN_VIEW_USER_SPELLS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_VIEW_USER_SKILLS to ", "", true, GOD_CAN_VIEW_USER_SKILLS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_VIEW_USER_SKILLS from ", "", false, GOD_CAN_VIEW_USER_SKILLS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_VIEW_USER_APPEARANCE_CORPSE to ", "", true, GOD_CAN_VIEW_USER_APPEARANCE_CORPSE },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_VIEW_USER_APPEARANCE_CORPSE from ", "", false, GOD_CAN_VIEW_USER_APPEARANCE_CORPSE },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_LOCKOUT_USER to ", "", true, GOD_CAN_LOCKOUT_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_LOCKOUT_USER from ", "", false, GOD_CAN_LOCKOUT_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_SLAY_USER to ", "", true, GOD_CAN_SLAY_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_SLAY_USER from ", "", false, GOD_CAN_SLAY_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_COPY_USER to ", "", true, GOD_CAN_COPY_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_COPY_USER from ", "", false, GOD_CAN_COPY_USER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EMULATE_MONSTER to ", "", true, GOD_CAN_EMULATE_MONSTER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EMULATE_MONSTER from ", "", false, GOD_CAN_EMULATE_MONSTER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_INVINCIBLE to ", "", true, GOD_INVINCIBLE },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_INVINCIBLE from ", "", false, GOD_INVINCIBLE },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_DEVELOPPER to ", "", true, GOD_DEVELOPPER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_DEVELOPPER from ", "", false, GOD_DEVELOPPER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_SHUTDOWN to ", "", true, GOD_CAN_SHUTDOWN },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_SHUTDOWN from ", "", false, GOD_CAN_SHUTDOWN },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_SEE_ACCOUNTS to ", "", true, GOD_CAN_SEE_ACCOUNTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_SEE_ACCOUNTS from ", "", false, GOD_CAN_SEE_ACCOUNTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_GIVE_GOD_FLAGS to ", "", true, GOD_CAN_GIVE_GOD_FLAGS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_GIVE_GOD_FLAGS from ", "", false, GOD_CAN_GIVE_GOD_FLAGS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_UNLIMITED_SHOUTS to ", "", true, GOD_UNLIMITED_SHOUTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_UNLIMITED_SHOUTS from ", "", false, GOD_UNLIMITED_SHOUTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_TRUE_INVISIBILITY to ", "", true, GOD_TRUE_INVISIBILITY },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_TRUE_INVISIBILITY from ", "", false, GOD_TRUE_INVISIBILITY },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_EMULATE_SYSTEM to ", "", true, GOD_CAN_EMULATE_SYSTEM },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_EMULATE_SYSTEM from ", "", false, GOD_CAN_EMULATE_SYSTEM },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CHAT_MASTER to ", "", true, GOD_CHAT_MASTER },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CHAT_MASTER from ", "", false, GOD_CHAT_MASTER },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CANNOT_DIE to ", "", true, GOD_CANNOT_DIE },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CANNOT_DIE from ", "", false, GOD_CANNOT_DIE },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_CAN_RUN_CLIENT_SCRIPTS to ", "", true, GOD_CAN_RUN_CLIENT_SCRIPTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_CAN_RUN_CLIENT_SCRIPTS from ", "", false, GOD_CAN_RUN_CLIENT_SCRIPTS },
   { GOD_CAN_GIVE_GOD_FLAGS, "give gameop flag GOD_BOOST_XP to ", "", true, GOD_BOOST_XP },
   { GOD_CAN_GIVE_GOD_FLAGS, "remove gameop flag GOD_BOOST_XP from ", "", false, GOD_BOOST_XP },
};


static void Client_CheckAdminCommand (client_t *client)
{
   // helper that parses the admin commands array and takes the relevant action

   int index;
   int prefix_length;
   admin_command_t *command;
   char target_name[32];
   client_t *target_client;

   // cycle through all admin commands...
   for (index = 0; index < sizeof (admin_commands) / sizeof (admin_command_t); index++)
   {
      command = &admin_commands[index]; // quick access to command

      if (!client->account_flags[command->required_flag])
         continue; // if client doesn't have the required flag for this command, skip it

      prefix_length = strlen (command->prefix); // get prefix length

      if (strnicmp (client->last_sent_message, command->prefix, prefix_length) != 0)
         continue; // if what the user typed is NOT this command, skip it

      if ((command->suffix != NULL) && (stristr (client->last_sent_message, command->suffix) == NULL))
         continue; // if what the user typed is NOT this command, skip it

      // get the target client
      SAFE_strncpy (target_name, client->last_sent_message + prefix_length, sizeof (target_name)); // get target name
      if (command->suffix != NULL)
         *stristr (target_name, command->suffix) = 0; // terminate string
      target_client = Client_GetByPlayerName (target_name); // get target client structure

      if (!Client_IsValid (target_client))
         continue; // if client is invalid, skip it

      // now either grant or withdraw the flag from our target
      if (command->must_grant)
         target_client->account_flags[command->granted_flag] = true; // flag granted
      else
         target_client->account_flags[command->granted_flag] = false; // flag withdrawn
   }

   return; // finished
}


bool ClientList_Init (void)
{
   // this function allocates space in memory for the clients list and return its success

   int index;

   // allocate list, zero it out and set its count to zero
   client_count = 0;
   clients = (client_t *) SAFE_malloc (server.config.general.max_clients, sizeof (client_t), true);

   if (clients == NULL)
      return (false); // memory allocation failed, return FALSE

   memset (hashtable, 0, sizeof (hashtable)); // reset hasthable if it isn't initialized yet

   // for all of them, reset their socket ID to INVALID_SOCKET
   for (index = 0; index < server.config.general.max_clients; index++)
      clients[index].socket_client = INVALID_SOCKET;

   return (true); // we succeeded, return TRUE
}


void ClientList_Shutdown (void)
{
   // this function frees the memory space that was allocated for the clients list

   int index;

   // for each client...
   for (index = 0; index < client_count; index++)
      Client_Disconnect (&clients[index]); // disconnect it cleanly

   // free the list and set its count to zero
   SAFE_free (&clients);
   client_count = 0;

   return; // finished
}


void Client_Rehash (client_t *client)
{
   // this function rehashs the specified client

   hashtable_bucket_t *bucket;
   int index;

   // get a quick access to this client's hashtable bucket
   bucket = &hashtable[client->hash];

   // first, draw this element from its hashtable
   for (index = 0; index < bucket->size; index++)
      if (bucket->entries[index].element_address == (void *) client)
         break; // break as soon as we find it
   if (index < bucket->size)
   {
      // if found, shift the rest of the hashtable one element down
      memmove (&bucket->entries[index], &bucket->entries[index + 1], (bucket->size - index - 1) * sizeof (hashtable_entry_t));
      bucket->entries = (hashtable_entry_t *) SAFE_realloc (bucket->entries, bucket->size, bucket->size - 1, sizeof (hashtable_entry_t), false);
      bucket->size--; // bucket has now one element less
   }

   // compute its new hash value and put it in the hashtable
   client->hash = Util_HashInteger (client->player_id, HASH_GRANULARITY);
   bucket = &hashtable[client->hash];

   // reallocate hashtable bucket, append element to it and increase bucket size
   bucket->entries = (hashtable_entry_t *) SAFE_realloc (bucket->entries, bucket->size, bucket->size + 1, sizeof (hashtable_entry_t), false);
   bucket->entries[bucket->size].element_address = (void *) client;
   bucket->size++; // bucket has now one element more

   return; // finished
}


int Client_Count (void)
{
   // this function returns the amount of client structures currently used in the client list

   return (client_count); // as simple as that
}


bool Client_IsValid (client_t *client)
{
   // this function returns true when the specified client slot is occupied by a valid player

   return ((client != NULL) && client->is_active && (client->account_name[0] != 0)
           && (client->player_name[0] != 0) && (client->player_id != 0));
}


int Client_IndexOf (client_t *client)
{
   // this function returns the index of the slot in the clients array occupied by client

   return (((unsigned long) client - (unsigned long) clients) / sizeof (client_t));
}


client_t *Client_GetByIndex (int client_index)
{
   // this function returns the address of the client structure whose position in the array of
   // client structures is client_index

   return (&clients[client_index]); // as simple as that
}


client_t *Client_GetByAccountName (const char *account_name)
{
   // this function returns the address of the client structure of the client who has the given
   // account name in the client's array of structure

   int index;
   int length;

   if ((account_name == NULL) || (account_name[0] == 0))
      return (NULL); // consistency check

   length = strlen (account_name); // get wanted account name's length

   // do we want autocompletion ?
   if (account_name[length - 1] == '+')
   {
      // for each connected client, compare its name with the account name we want...
      for (index = 0; index < client_count; index++)
         if (strnicmp (clients[index].account_name, account_name, length - 1) == 0)
            return (&clients[index]); // found it, return a pointer to the beginning of the struct
   }
   else
   {
      // for each connected client, compare its name with the account name we want...
      for (index = 0; index < client_count; index++)
         if (stricmp (clients[index].account_name, account_name) == 0)
            return (&clients[index]); // found it, return a pointer to the beginning of the struct
   }

   return (NULL); // client not found
}


client_t *Client_GetByPlayerName (const char *player_name)
{
   // this function returns the address of the client structure of the client who has the given
   // player name in the client's array of structure

   int index;
   int length;

   if ((player_name == NULL) || (player_name[0] == 0))
      return (NULL); // consistency check

   length = strlen (player_name); // get wanted account name's length

   // do we want autocompletion ?
   if (player_name[length - 1] == '+')
   {
      // for each connected client, compare its name with the player name we want...
      for (index = 0; index < client_count; index++)
         if (strnicmp (clients[index].player_name, player_name, length - 1) == 0)
            return (&clients[index]); // found it, return a pointer to the beginning of the struct
   }
   else
   {
      // for each connected client, compare its name with the player name we want...
      for (index = 0; index < client_count; index++)
         if (stricmp (clients[index].player_name, player_name) == 0)
            return (&clients[index]); // found it, return a pointer to the beginning of the struct
   }

   return (NULL); // client not found
}


client_t *Client_GetByPlayerId (int player_id)
{
   // this function returns the address of the client structure of the client who has the given
   // player ID in the client's array of structure

   hashtable_bucket_t *bucket;
   int index;

   // get the hash value of the ID number we want and its corresponding hashtable bucket
   bucket = &hashtable[Util_HashInteger (player_id, HASH_GRANULARITY)];

   // cycle through all clients we know and return the one that has the right id number
   for (index = 0; index < bucket->size; index++)
      if (((client_t *) bucket->entries[index].element_address)->player_id == player_id)
         return ((client_t *) bucket->entries[index].element_address); // found it

   return (NULL); // client not found
}


client_t *Client_GetByNetworkAddress (address_t *client_address)
{
   // this function returns the address of the client structure of the client who has the given
   // IP address in the client's array of structure

   int index;

   // for each connected client, compare its address with the IP address we want...
   for (index = 0; index < client_count; index++)
      if ((clients[index].client_address.port == client_address->port)
          && (strcmp (clients[index].client_address.ip, client_address->ip) == 0))
         return (&clients[index]); // found it, return a pointer to the beginning of the struct

   return (NULL); // client not found
}


void Client_Connect (client_t *client, datagram_t *datagram)
{
   // this function sets up a new client's structure using remote_service's connection data

   hashtable_bucket_t *bucket;
   int checksum_startpos;
   char checksum_bytes[32];
   int checksum;
   int index;

   // first off, cycle through all existing clients
   for (index = 0; index < client_count; index++)
   {
      // is the client port he intends to use already in use ?
      if (clients[index].client_address.port == datagram->peer_address.port)
      {
         Log_Text (LOG_MAIN, "Server_ListenToClients(): Peer %s:%d attempted to connect but this client port is already in use by another client. Preventing connection.\n", datagram->peer_address.ip, datagram->peer_address.port);
         return; // if so, refuse connection and don't handle this packet
      }
   }

   // do we require XTEA encryption ?
   if (server.config.general.want_encryption)
   {
      // is datagram too short to contain the game executable checksum ?
      if (datagram->size < (short) strlen (T4CGUARD_NAME " " T4CGUARD_VERSION) + 1)
      {
         Log_Text (LOG_MAIN, "Client_Connect(): Peer %s:%d attempted to connect without specifying client version!.\n", datagram->peer_address.ip, datagram->peer_address.port);
         return;
      }

      client->uses_encryption = true; // this client uses T4C Guard encryption

      // the client version is given by its executable's checksum, so pick it
      checksum_startpos = strlen (T4CGUARD_NAME " " T4CGUARD_VERSION) + 1;
      memcpy (checksum_bytes, datagram->bytes + checksum_startpos, datagram->size - checksum_startpos);
      checksum_bytes[datagram->size - checksum_startpos] = 0;
      checksum = atoi (checksum_bytes);
      if ((checksum == T4CCLIENT_EXE_CHECKSUM_125) || (checksum == T4CCLIENT_EXE_CHECKSUM_125h))
         client->game_version = PAKCRYPTO_VERSION_125;
      else if (checksum == T4CCLIENT_EXE_CHECKSUM_140)
         client->game_version = PAKCRYPTO_VERSION_140;
      else if ((checksum == T4CCLIENT_EXE_CHECKSUM_150) || (checksum == T4CCLIENT_EXE_CHECKSUM_160b))
         client->game_version = PAKCRYPTO_VERSION_150;
      else if (checksum == T4CCLIENT_EXE_CHECKSUM_161)
         client->game_version = PAKCRYPTO_VERSION_161;
      else if ((checksum == T4CCLIENT_EXE_CHECKSUM_163) || (checksum == T4CCLIENT_EXE_CHECKSUM_163h) || (checksum == T4CCLIENT_EXE_CHECKSUM_163h_old))
         client->game_version = PAKCRYPTO_VERSION_163;
      else
      {
         Log_Text (LOG_MAIN, "Client_Connect(): Peer %s:%d attempted to connect with unsupported client version (checksum=%d).\n", datagram->peer_address.ip, datagram->peer_address.port, checksum);
         memset (client, 0, sizeof (client_t)); // so kick him out
         client->socket_client = INVALID_SOCKET;
         return;
      }

      // do we require a particular game version AND is it NOT high enough ?
      if ((server.config.general.wanted_version != 0) && (client->game_version < server.config.general.wanted_version))
      {
         Log_Text (LOG_MAIN, "Client_Connect(): Peer %s:%d attempted to connect with forbidden client version (his version=%d, requested version=%d).\n", datagram->peer_address.ip, datagram->peer_address.port, client->game_version, server.config.general.wanted_version);
         memset (client, 0, sizeof (client_t)); // so kick him out
         client->socket_client = INVALID_SOCKET;
         return;
      }
   }
   else
   {
      // check if this client is sending a NON-encrypted datagram : the first pak the client sends,
      // under XTEA encryption, is 8 bytes longer than usual.
      if (datagram->size == 18)
      {
         // is this protocol not allowed ?
         if ((server.config.general.wanted_version != 0) && (server.config.general.wanted_version > PAKCRYPTO_VERSION_125))
         {
            Log_Text (LOG_MAIN, "Client_Connect(): Peer %s:%d attempted to connect with wrong protocol version (his=125, required=%d).\n", datagram->peer_address.ip, datagram->peer_address.port, server.config.general.wanted_version);
            memset (client, 0, sizeof (client_t)); // so kick him out
            client->socket_client = INVALID_SOCKET;
            return;
         }

         client->uses_encryption = false; // this client uses no encryption and we allow it
         client->game_version = PAKCRYPTO_VERSION_125;
      }
      else if (datagram->size == 20)
      {
         // is this protocol not allowed ?
         if ((server.config.general.wanted_version != 0) && (server.config.general.wanted_version > PAKCRYPTO_VERSION_150))
         {
            Log_Text (LOG_MAIN, "Client_Connect(): Peer %s:%d attempted to connect with wrong protocol version (his=150, required=%d).\n", datagram->peer_address.ip, datagram->peer_address.port, server.config.general.wanted_version);
            memset (client, 0, sizeof (client_t)); // so kick him out
            client->socket_client = INVALID_SOCKET;
            return;
         }

         client->uses_encryption = false; // this client uses no encryption and we allow it
         client->game_version = PAKCRYPTO_VERSION_150;
      }
   }

   // now we know whether client uses crypto or not and which version of it

   // remember client address
   SAFE_strncpy (client->client_address.ip, datagram->peer_address.ip, 16);
   client->client_address.port = datagram->peer_address.port;

   // create the server to proxy output socket
   if ((client->socket_client = SenderReceiver_New (datagram->peer_address.port + 1)) == INVALID_SOCKET)
   {
      Log_Text (LOG_MAIN, "Client_Connect(): Failed creating new client socket: %s\n", Util_GetLastNetworkError ());
      memset (client, 0, sizeof (client_t)); // on error, cleanup and return
      client->socket_client = INVALID_SOCKET;
      return;
   }

   // set this fake client service parameters
   SAFE_strncpy (client->service_fakeclient.ip, datagram->peer_address.ip, 16);
   client->service_fakeclient.port = datagram->peer_address.port;

   // notify that we've registered a new peer
   if (server.config.log.mainlog.level > 1)
      Log_Text (LOG_MAIN, "Client_Connect(): Registered peer %s:%d (%sencryption) - game version: %d.\n", client->service_fakeclient.ip, client->service_fakeclient.port, (client->uses_encryption ? "" : "NO "), client->game_version);

   // this client's alive, remember it
   client->is_active = true;
   client->last_activity = server.timers.current_time;

   // client has not been given any player ID yet, set him a default one so he can be hashed
   client->player_id = 0;

   // compute its hash value and put it in the hashtable
   client->hash = Util_HashInteger (client->player_id, HASH_GRANULARITY);
   bucket = &hashtable[client->hash];

   // reallocate hashtable bucket, append element to it and increase bucket size
   bucket->entries = (hashtable_entry_t *) SAFE_realloc (bucket->entries, bucket->size, bucket->size + 1, sizeof (hashtable_entry_t), false);
   bucket->entries[bucket->size].element_address = (void *) client;
   bucket->size++; // bucket has now one element more

   // is this client slot beyond the ones we know ?
   if (Client_IndexOf (client) == client_count)
      client_count++; // if so, remember our array holds one client structure more

   return; // client structure is ready for use
}


void Client_Disconnect (client_t *client)
{
   // this function frees the resources allocated with a client structure and erases it

   hashtable_bucket_t *bucket;
   pak_t *new_pak;
   int index;
   char date_string[32];
   plugin_t *plugin;
   int plugin_count;

   // has a socket been allocated for this client ?
   if (client->socket_client != INVALID_SOCKET)
   {
      // notify that we're dropping a peer
      if (server.config.log.mainlog.level > 1)
         Log_Text (LOG_MAIN, "Client_Disconnect(): Dropping peer %s:%d. Reason: %s\n", client->service_fakeclient.ip, client->service_fakeclient.port, (client->shutdown_requested ? "Shutdown requested" : "Client timeout"));

      // get a quick access to this client's hashtable bucket
      bucket = &hashtable[client->hash];

      // first, draw this element from its hashtable
      for (index = 0; index < bucket->size; index++)
         if (bucket->entries[index].element_address == (void *) client)
            break; // break as soon as we find it
      if (index < bucket->size)
      {
         // if found, shift the rest of the hashtable one element down
         memmove (&bucket->entries[index], &bucket->entries[index + 1], (bucket->size - index - 1) * sizeof (hashtable_entry_t));
         bucket->entries = (hashtable_entry_t *) SAFE_realloc (bucket->entries, bucket->size, bucket->size - 1, sizeof (hashtable_entry_t), false);
         bucket->size--; // bucket has now one element less
      }

      // record this account's IP in the database if client has a known account
      if (client->account_name[0] != 0)
      {
         // format the date and convert it to the "DD/MM/YYYY\0" format
         _strdate (date_string);
         date_string[24] = date_string[0]; // save month
         date_string[25] = date_string[1];
         date_string[0] = date_string[3]; // move day
         date_string[1] = date_string[4];
         date_string[3] = date_string[24]; // move month
         date_string[4] = date_string[25];
         date_string[8] = date_string[6]; // mode decade
         date_string[9] = date_string[7];
         date_string[6] = '2'; // put millenium
         date_string[7] = '0';
         date_string[10] = 0; // and terminate string

         // prepare a safe SQL string
         SQL_MakeStringSafe (client->account_name, sizeof (client->account_name), false);
         SQL_ExecuteSingleRequest (true, "UPDATE UserAccounts SET LastVisit='%s',LastIP='%s' WHERE Account='%s'", date_string, client->service_fakeclient.ip, client->account_name);

         // prepare a new "client disconnection" pak
         new_pak = Pak_New (0, PAKFLAG_NONE);
         Pak_AddInt16 (new_pak, PAK_CLIENT_ExitGame);
         Client_SendPakToServer (client, new_pak); // send it to the server
         Pak_Destroy (new_pak);

         // did client NOT request the shutdown himself ? if so, tell him we're disconnecting him
         if (!client->shutdown_requested)
            Server_SendMessageToClient (client, "Des difficultés sur le réseau ont réinitialisé votre connexion au serveur. Veuillez vous reconnecter.");

         // if client has a player, notify all the plugins that this client just disconnected
         if (Client_IsValid (client))
         {
            plugin_count = Plugin_Count (); // get plugin count and cycle through all of them
            for (index = 0; index < plugin_count; index++)
            {
               plugin = Plugin_GetByIndex (index); // quick access to plugin

               if (!Plugin_IsValid (plugin))
                  continue; // skip invalid plugin slots

               plugin->PluginClientDisconnect (client); // notify plugin client is disconnecting
            }
         }

         // free this client's dynamically allocated data
         SAFE_free (&client->skills);
         SAFE_free (&client->spells);
         SAFE_free (&client->items);
      }

      SenderReceiver_Destroy (&client->socket_client); // and close this client's socket
   }
   memset (client, 0, sizeof (client_t)); // and completely reset this client slot
   client->socket_client = INVALID_SOCKET;

   return; // we can now forget this client
}


bool Client_InterceptComms (client_t *client, pak_t *pak)
{
   // this function parses the pak in order to know whether there is a client command to execute

   // WARNING: THIS INTERFACE OF THE PROXY IS EXPOSED TO MALICIOUS PAK ATTACKS, SO CHECK ANY DATA
   // BEFORE EXTRACTING IT FROM THE NETWORK!

   unsigned short pak_type;
   char command[256];
   char arg1[256];
   char arg2[256];
//   char arg3[256];
   int index;
   int index2;
   int length;
   int length2;
//   int count;
//   pak_t *new_pak;
//   char *target_client_name;
//   char *webpatch_urlparser;
   client_t *target_client;
   plugin_t *plugin;
   int plugin_count;
   unit_t *unit;
//   void *db_statement;
//   FILE *fp;

   // get the pak type
   pak_type = Pak_ReadInt16 (pak, 0);

   // given the type of pak it is, retrieve some info about the client

   // movement request ?
   if (pak_type == PAK_CLIENT_PlayerMove_N)
      client->orientation = ORIENTATION_NORTH;
   else if (pak_type == PAK_CLIENT_PlayerMove_NE)
      client->orientation = ORIENTATION_NORTHEAST;
   else if (pak_type == PAK_CLIENT_PlayerMove_E)
      client->orientation = ORIENTATION_EAST;
   else if (pak_type == PAK_CLIENT_PlayerMove_SE)
      client->orientation = ORIENTATION_SOUTHEAST;
   else if (pak_type == PAK_CLIENT_PlayerMove_S)
      client->orientation = ORIENTATION_SOUTH;
   else if (pak_type == PAK_CLIENT_PlayerMove_SW)
      client->orientation = ORIENTATION_SOUTHWEST;
   else if (pak_type == PAK_CLIENT_PlayerMove_W)
      client->orientation = ORIENTATION_WEST;
   else if (pak_type == PAK_CLIENT_PlayerMove_NW)
      client->orientation = ORIENTATION_NORTHWEST;

/*   // message of the day ?
   else if (pak_type == PAK_CLIENT_MessageOfTheDay)
   {
      // read the MOTD file
      fp = fopen (server.config.t4cserver.motd_pathname, "rb");
      if (fp != NULL)
      {
         length = fread (motd_text, 1, sizeof (motd_text) - 1, fp); // read the file contents
         motd_text[length] = 0; // ensure string is terminated
         fclose (fp); // finished, close the file
      }
      else
         SAFE_strncpy (motd_text, server.config.general.welcome_message, sizeof (motd_text)); // else put default text

      // prepare a new message of the day pak
      new_pak = Pak_New (0, PAKFLAG_NONE);
      Pak_AddInt16 (new_pak, PAK_SERVER_MessageOfTheDay);
      Pak_AddString16 (new_pak, motd_text);
      Server_SendPakToClient (client, new_pak); // and send it to the client
      Pak_Destroy (new_pak);

      return (true); // we handled this pak
   }
*/
/*   // patch server information ?
   else if (pak_type == PAK_CLIENT_PatchServerInformation)
   {
      // prepare a new patch server information pak
      new_pak = Pak_New (0, PAKFLAG_NONE);
      Pak_AddInt16 (new_pak, PAK_SERVER_PatchServerInformation);
      Pak_AddInt32 (new_pak, server.config.general.wanted_version);

      // find where the webpatch relative path starts and copy it
      webpatch_urlparser = strstr (server.config.webpatch.url, "://");
      if (webpatch_urlparser != NULL)
         webpatch_urlparser += 3;
      else
         webpatch_urlparser = server.config.webpatch.url;
      webpatch_urlparser = strchr (webpatch_urlparser, '/');
      if (webpatch_urlparser == NULL)
         webpatch_urlparser = "";
      SAFE_strncpy (arg2, webpatch_urlparser, sizeof (arg2));

      // find the webpatch URL and copy it
      webpatch_urlparser = strstr (server.config.webpatch.url, "://");
      if (webpatch_urlparser != NULL)
         webpatch_urlparser += 3;
      else
         webpatch_urlparser = server.config.webpatch.url;
      SAFE_strncpy (arg1, webpatch_urlparser, sizeof (arg1));
      webpatch_urlparser = strchr (arg1, '/');
      if (webpatch_urlparser != NULL)
         webpatch_urlparser[0] = 0;

      Pak_AddString16 (new_pak, arg1); // url
      Pak_AddString16 (new_pak, arg2); // filename
      Pak_AddString16 (new_pak, server.config.webpatch.login); // login
      Pak_AddString16 (new_pak, server.config.webpatch.password); // password
      Pak_AddInt16 (new_pak, 2); // unknown value

      Server_SendPakToClient (client, new_pak); // and send it to the client
      Pak_Destroy (new_pak);

      return (true); // we handled this pak
   }
*/
   // login request ?
   else if (pak_type == PAK_CLIENT_Login)
   {
      length = Pak_ReadInt8 (pak, 2); // get string lengths
      length2 = Pak_ReadInt8 (pak, 2 + 1 + length);

      // is the player trying to crash the server ?
      if (pak->data_size != 2 + 1 + length + 1 + length2 + 2 + 2)
      {
         Log_Pak (LOG_MAIN, pak, "Client_InterceptComms(): ALERT: Client %s:%d attempted to crash server using incorrect pak data size. Ignoring pak.\n", client->client_address.ip, client->client_address.port);
         return (true); // log the cheating attempt and drop this pak
      }

      // read client's claimed account and password (i.e, name under which he tries to login)
      Pak_ReadString8 (pak, 2, arg1, 32);
      SQL_MakeStringSafe (arg1, 32, false); // make it SQL safe
      Pak_ReadString8 (pak, 2 + 1 + length, arg2, 32);
      SQL_MakeStringSafe (arg2, 32, false); // make it SQL safe
      index = Pak_ReadInt16 (pak, 2 + 1 + length + 1 + length2); // read client version
      index2 = Pak_ReadInt16 (pak, 2 + 1 + length + 1 + length2 + 2); // read unknown

      // now rewrite the pak with the cleaned up and case-lowered strings
      Pak_WriteString8 (pak, true, 2, arg1, 32);
      Pak_AddString8 (pak, arg2);
      Pak_AddInt16 (pak, (int16) index);
      Pak_AddInt16 (pak, (int16) index2);

      // save this client's claimed account name
      SAFE_strncpy (client->claimed_account_name, arg1, sizeof (client->claimed_account_name));

      return (false); // forward this pak to the server
   }

/*   // client version authentication request ?
   else if (pak_type == PAK_CLIENT_AuthenticateVersion)
   {
      // prepare a new "version [not] authenticated" pak
      new_pak = Pak_New (0, PAKFLAG_NONE);
      Pak_AddInt16 (new_pak, PAK_SERVER_AuthenticateVersion);
      Pak_AddInt32 (new_pak, (Pak_ReadInt32 (pak, 2) == 125));
      Server_SendPakToClient (client, new_pak); // and send it to the client
      Pak_Destroy (new_pak);

// DON'T DROP THIS PAK, TRANSMIT IT TO THE SERVER ANYWAY
//      return (true); // pak handled

      return (false); // forward this pak to the server
   }
*/
/*   // character list request ?
   else if (pak_type == PAK_CLIENT_PersonalPCList)
   {
      // prepare a new "character list" pak
      new_pak = Pak_New (0, PAKFLAG_NONE);
      Pak_AddInt16 (new_pak, PAK_SERVER_PersonalPCList);
      Pak_AddInt8 (new_pak, 0); // we'll update the count later

      // get this client's characters from the database
      count = 0;
      db_statement = SQL_AllocateStatement ();
      if (SQL_ExecuteRequest (db_statement, false, "SELECT PlayerName,Appearance,CurrentLevel FROM PlayingCharacters WHERE AccountName='%s'", client->account_name))
      {
         SQL_BindColumnToString (db_statement, true, 0, arg1, sizeof (arg1));
         SQL_BindColumnToString (db_statement, true, 1, arg2, sizeof (arg2));
         SQL_BindColumnToString (db_statement, true, 2, arg3, sizeof (arg3));
         while (SQL_ReadLine (db_statement, true))
         {
            Pak_AddString8 (new_pak, arg1); // player name
            Pak_AddInt16 (new_pak, (int16) atoi (arg2)); // skin ID
            Pak_AddInt16 (new_pak, (int16) atoi (arg3)); // level
            count++; // one character more
         }
         SQL_CloseCursor (db_statement, true);
      }
      SQL_FreeStatement (db_statement);

      Pak_WriteInt8 (new_pak, false, 2, (int8) count); // update character count
      Server_SendPakToClient (client, new_pak); // and send it to the client
      Pak_Destroy (new_pak);

// DON'T DROP THIS PAK, TRANSMIT IT TO THE SERVER ANYWAY
//      return (true); // pak handled

      return (false); // forward this pak to the server
   }
*/
   // player creation request ?
   else if (pak_type == PAK_CLIENT_CreatePlayer)
   {
      length = Pak_ReadString8 (pak, 8, arg1, sizeof (arg1)); // get string length

      // is the player trying to crash the server ?
      if (pak->data_size != 2 + 5 + 1 + 1 + length)
      {
         Log_Pak (LOG_MAIN, pak, "Client_InterceptComms(): ALERT: Client %s:%d (%s:%s) attempted to crash server using incorrect pak data size. Ignoring pak.\n", client->client_address.ip, client->client_address.port, client->account_name, client->player_name);
         return (true); // log the cheating attempt and drop this pak
      }

      // for each character in string, convert its apostrophes and check for possible malformation
      SQL_MakeStringSafe (arg1, sizeof (arg1), false);

      // now verify the presence of capital letters at the beginning of each word
      index2 = true;
      for (index = 0; index < length; index++)
      {
         // does this letter need to be capitalized ?
         if (index2)
         {
            arg1[index] = toupper (arg1[index]); // ensure it is
            index2 = false; // next one doesn't
         }
         else
            arg1[index] = tolower (arg1[index]); // ensure it is

         // is this character a separator ?
         if (((arg1[index] == ' ')
              && (strnicmp (&arg1[index + 1], "de ", 3) != 0)
              && (strnicmp (&arg1[index + 1], "du ", 3) != 0)
              && (strnicmp (&arg1[index + 1], "d’", 2) != 0)
              && (strnicmp (&arg1[index + 1], "d ", 2) != 0))
             || (arg1[index] == '-'))
            index2 = true; // if so, next letter needs to be capitalized
      }

      // and finally, put it back in the pak
      Pak_WriteString8 (pak, false, 8, arg1);

      return (false); // forward this pak to the server
   }
   else if (pak_type == PAK_CLIENT_PutPlayerInGame)
   {
      length = Pak_ReadInt8 (pak, 2); // get string lengths

      // is the player trying to crash the server ?
      if (pak->data_size != 2 + 1 + length)
      {
         Log_Pak (LOG_MAIN, pak, "Client_InterceptComms(): ALERT: Client %s:%d (%s:%s) attempted to crash server using incorrect pak data size. Ignoring pak.\n", client->client_address.ip, client->client_address.port, client->account_name, client->player_name);
         return (true); // log the cheating attempt and drop this pak
      }

      Pak_ReadString8 (pak, 2, client->player_name, sizeof (client->player_name)); // read the client's player name

      return (false); // forward this pak to the server
   }
   else if (pak_type == PAK_CLIENT_ExitGame)
   {
      Log_Text (LOG_MAIN, "Client_InterceptComms(): Player '%s' shutting down with account '%s' from IP %s:%d.\n", client->player_name, client->account_name, client->service_fakeclient.ip, client->service_fakeclient.port);
      client->shutdown_requested = true; // flag this client as leaving the game

      return (false); // forward this pak to the server
   }
   else if (pak_type == PAK_CLIENT_UnitName)
   {
      index = Pak_ReadInt32 (pak, 2); // read unit ID
      if (index < 0)
         return (true); // shit happens (wtf ?) relatively often. Client bug ?

      client->last_targeted_unit_id = index; // save unit ID
      client->last_targeted_time = server.timers.current_time; // remember date

      // did unit change ? if so, find the new one
      if (!Unit_IsValid (client->last_targeted_unit)
          || (client->last_targeted_unit_id != client->last_targeted_unit->unit_id))
         client->last_targeted_unit = Unit_FindSlotForId (client->last_targeted_unit_id);

      return (false); // forward this pak to the server
   }
   else if (pak_type == PAK_CLIENT_ItemDescription)
   {
      unit = Unit_FindSlotForId (Pak_ReadInt32 (pak, 2)); // get the unit it is
      if (Unit_IsValid (unit))
         Unit_EnsureIdentificationByClient (unit, client); // ensure unit identification

      return (false); // forward this pak to the server
   }
   else if (pak_type == PAK_CLIENT_DirectedTalk)
   {
      length = Pak_ReadInt16 (pak, 15); // get message length

      // is the player trying to crash the server ?
      if (pak->data_size != 2 + 2 + 2 + 4 + 1 + 4 + 2 + length)
      {
         Log_Pak (LOG_MAIN, pak, "Client_InterceptComms(): ALERT: Client %s:%d (%s:%s) attempted to crash server using incorrect pak data size. Ignoring pak.\n", client->client_address.ip, client->client_address.port, client->account_name, client->player_name);
         return (true); // log the cheating attempt and drop this pak
      }

      client->last_targeted_unit_id = Pak_ReadInt32 (pak, 6); // read unit ID
      client->last_targeted_time = server.timers.current_time; // remember date

      // did unit change ? if so, find the new one
      if (!Unit_IsValid (client->last_targeted_unit)
          || (client->last_targeted_unit_id != client->last_targeted_unit->unit_id))
         client->last_targeted_unit = Unit_FindSlotForId (client->last_targeted_unit_id);

      client->last_directedtalk_unit = client->last_targeted_unit; // remember NPC player was talking to
      client->last_directedtalk_time = server.timers.current_time; // and date at which it was
      client->orientation = Pak_ReadInt8 (pak, 10); // read player's orientation
      Pak_ReadString16 (pak, 15, client->last_sent_message, sizeof (client->last_sent_message));

      return (false); // forward this pak to the server
   }
   else if (pak_type == PAK_CLIENT_BroadcastTextChange)
   {
      client->last_directedtalk_unit_id = Pak_ReadInt32 (pak, 2); // read unit ID
      client->last_directedtalk_time = server.timers.current_time; // remember date

      // did unit change ? if so, find the new one
      if (!Unit_IsValid (client->last_directedtalk_unit)
          || (client->last_directedtalk_unit_id != client->last_directedtalk_unit->unit_id))
         client->last_directedtalk_unit = Unit_FindSlotForId (client->last_directedtalk_unit_id);

      return (false); // forward this pak to the server
   }
   else if (pak_type == PAK_CLIENT_IndirectTalk)
   {
      length = Pak_ReadInt16 (pak, 11); // get string length

      // is the player trying to crash the server ?
      if (pak->data_size != 2 + 4 + 1 + 4 + 2 + length)
      {
         Log_Pak (LOG_MAIN, pak, "Client_InterceptComms(): ALERT: Client %s:%d (%s:%s) attempted to crash server using incorrect pak data size. Ignoring pak.\n", client->client_address.ip, client->client_address.port, client->account_name, client->player_name);
         return (true); // log the cheating attempt and drop this pak
      }

      // remember the message the client typed
      Pak_ReadString16 (pak, 11, client->last_sent_message, sizeof (client->last_sent_message));

      // check if it is an admin command, and if so, execute it
      Client_CheckAdminCommand (client);

      // is this client renaming somebody AND is he allowed to ?
      if (client->account_flags[GOD_CAN_EDIT_USER_NAME] && (strnicmp (client->last_sent_message, "set ", 4) == 0) && (stristr (client->last_sent_message, "'s name to ") != NULL))
      {
         SAFE_strncpy (arg1, client->last_sent_message + 4, sizeof (arg1)); // get target name
         *strchr (arg1, '\'') = 0; // terminate string
         target_client = Client_GetByPlayerName (arg1); // get target client structure
         if (Client_IsValid (target_client))
            SAFE_strncpy (target_client->player_name, stristr (client->last_sent_message, "'s name to ") + 11, sizeof (target_client->player_name));
      }

      // don't go further if it ain't a proxy command
      if (client->last_sent_message[0] != '.')
         return (false);

      // it is a proxy command, so get command and arguments
      Util_GetWord (client->last_sent_message, 0, command, sizeof (command));
      Util_GetWord (client->last_sent_message, 1, arg1, sizeof (arg1));
      Util_GetWord (client->last_sent_message, 2, arg2, sizeof (arg2));

      // is the user requesting for help ?
      if ((stricmp (command, ".help") == 0) || (stricmp (command, ".?") == 0))
      {
         // have we requested help for no particular plugin ?
         if ((arg1 == NULL) || (arg1[0] == 0))
         {
            // print a help message
            Server_SendMessageToClient (client, "\"" T4CGUARD_NAME "\" v\"" T4CGUARD_VERSION "\" par " T4CGUARD_AUTHOR);
            Server_SendMessageToClient (client, "Commandes disponibles:");
            Server_SendMessageToClient (client, "   {.help} {<plugin>} - Affiche l'aide du plugin. Si aucun plugin spécifié, affiche cet écran");

            // is the client NEITHER an administrator NOR a developper ?
            if (!client->account_flags[GOD_CAN_GIVE_GOD_FLAGS] && !client->account_flags[GOD_DEVELOPPER])
               return (true); // enough processing

            Server_SendMessageToClient (client, "   {.plugin_list} - Affiche la liste des plugins chargés");
            Server_SendMessageToClient (client, "   {.plugin_attach} {<plugin_path>} - Charge un plugin particulier");
            Server_SendMessageToClient (client, "   {.plugin_detach} {<plugin_path>} - Décharge un plugin particulier");
            Server_SendMessageToClient (client, "   {.update} {<proxy|plugins>} - Met à jour le proxy ou les plugins (à utiliser avec précaution !)");
            Server_SendMessageToClient (client, "   Les plugins chargés dans " T4CGUARD_NAME " proposent généralement des commandes supplémentaires ; reportez-vous à la documentation de vos plugins pour obtenir de l'aide sur celles-ci.");
         }
         else
         {
            // find the plugin(s) we want
            index2 = 0; // index2 will hold whether we found a plugin or not
            plugin_count = Plugin_Count (); // get plugin count and cycle through all of them
            for (index = 0; index < plugin_count; index++)
            {
               plugin = Plugin_GetByIndex (index); // quick access to plugin

               if (!Plugin_IsValid (plugin))
                  continue; // skip invalid plugin slots

               // does the name of this plugin bear a sort of resemblance with the one we want ?
               if (stristr (plugin->path, arg1) != NULL)
               {
                  // try to detach the plugin, and notify the user of it
                  Server_SendMessageToClient (client, "Aide du plugin \"%s\":", plugin->path);
                  plugin->PluginDisplayHelp (client); // display its help text
                  index2++; // remember we found one plugin more
               }
            }

            // have we found NO matching plugin ?
            if (index2 == 0)
               Server_SendMessageToClient (client, "Aucun plugin dont le nom contient \"%s\" n'a été trouvé", arg1);
         }

         return (true); // don't allow this pak to get to the server, we handle it
      }

      // else has the user requested the list of plugins ?
      else if (stricmp (command, ".plugin_list") == 0)
      {
         // is the client NEITHER an administrator NOR a developper ?
         if (!client->account_flags[GOD_CAN_GIVE_GOD_FLAGS] && !client->account_flags[GOD_DEVELOPPER])
            return (true); // enough processing

         // cycle through all plugins and display those who are loaded
         Server_SendMessageToClient (client, "Plugins actuellement chargés :");
         index2 = 0;
         plugin_count = Plugin_Count (); // get plugin count and cycle through all of them
         for (index = 0; index < plugin_count; index++)
         {
            plugin = Plugin_GetByIndex (index); // quick access to plugin

            if (!Plugin_IsValid (plugin))
               continue; // skip invalid plugin slots

            // we found a valid plugin, print info about it
            Server_SendMessageToClient (client, "   \"%s\" ver. %s - {%s <%s>}", plugin->name, plugin->version, plugin->author_name, plugin->author_email);
            index2++; // we found one plugin more
         }
         Server_SendMessageToClient (client, "%d plugin%s", index2, (index2 > 1 ? "s" : ""));

         return (true); // don't allow this pak to get to the server, we handle it
      }

      // else has the user requested to load a particular plugin ?
      else if (stricmp (command, ".plugin_attach") == 0)
      {
         // is the client NEITHER an administrator NOR a developper ?
         if (!client->account_flags[GOD_CAN_GIVE_GOD_FLAGS] && !client->account_flags[GOD_DEVELOPPER])
            return (true); // enough processing

         // try to load the plugin, and display whether we were successful or not
         if ((arg1 == NULL) || (arg1[0] == 0))
            Server_SendMessageToClient (client, "Veuillez spécifier le chemin du plugin à charger");
         else if (Plugin_Attach (arg1))
            Server_SendMessageToClient (client, "Plugin \"%s\" chargé avec succès", arg1);
         else
            Server_SendMessageToClient (client, "Impossible de charger le plugin \"%s\"", arg1);

         return (true); // don't allow this pak to get to the server, we handle it
      }

      // else has the user requested to unload a particular plugin ?
      else if (stricmp (command, ".plugin_detach") == 0)
      {
         // is the client NEITHER an administrator NOR a developper ?
         if (!client->account_flags[GOD_CAN_GIVE_GOD_FLAGS] && !client->account_flags[GOD_DEVELOPPER])
            return (true); // enough processing

         // try to unload specified plugins, and display whether we were successful or not
         if ((arg1 == NULL) || (arg1[0] == 0))
            Server_SendMessageToClient (client, "Veuillez spécifier le nom du plugin à décharger");
         else
         {
            // find the plugin(s) we want
            index2 = 0; // index2 will hold whether we found a plugin or not
            plugin_count = Plugin_Count (); // get plugin count and cycle through all of them
            for (index = 0; index < plugin_count; index++)
            {
               plugin = Plugin_GetByIndex (index); // quick access to plugin

               if (!Plugin_IsValid (plugin))
                  continue; // skip invalid plugin slots

               // does this plugin's name bear a sort of resemblance with the name we want ?
               if (stristr (plugin->path, arg1) != NULL)
               {
                  // try to detach the plugin, and notify the user of it
                  Server_SendMessageToClient (client, "Tentative de déchargement du plugin \"%s\"...", plugin->path);
                  index2 = 1;
                  if (Plugin_Detach (plugin->path))
                     Server_SendMessageToClient (client, "Plugin déchargé");
                  else
                     Server_SendMessageToClient (client, "Impossible de décharger le plugin");
               }
            }

            // have we found NO matching plugin ?
            if (index2 == 0)
               Server_SendMessageToClient (client, "Aucun plugin dont le nom contient \"%s\" n'a été trouvé", arg1);
         }

         return (true); // don't allow this pak to get to the server, we handle it
      }

      // else has the user requested a proxy update ?
      else if (stricmp (command, ".update") == 0)
      {
         // run the update script for either the whole proxy or the plugins
         // do we want to update the proxy of the plugins ?
         if ((arg1 != NULL) && (stricmp (arg1, "proxy") == 0))
         {
            // check for the availability of an update script
            if (server.config.general.updatescript_filename[0] != 0)
            {
               update_proxy = true; // raise the flag and notify the user that we're doing it
               Server_SendMessageToAllClients ("Proxy en cours de mise à jour, vous pourrez vous reconnecter dans quelques secondes...");
            }
            else
               Server_SendMessageToClient (client, "Commande non disponible : aucun script de mise à jour n'a été défini dans le fichier de configuration.");
         }
         else if ((arg1 != NULL) && (stricmp (arg1, "plugins") == 0))
         {
            // check for the availability of an update script
            if (server.config.general.updatescript_filename[0] != 0)
            {
               spawnl (P_NOWAIT, server.config.general.updatescript_filename, server.config.general.updatescript_filename, "plugins", NULL);
               Server_SendMessageToClient (client, "Plugins en cours de mise à jour, veuillez patienter quelques secondes...");
            }
            else
               Server_SendMessageToClient (client, "Commande non disponible : aucun script de mise à jour n'a été défini dans le fichier de configuration.");
         }
         else
            Server_SendMessageToClient (client, "Veuillez spécifier \"proxy\" ou \"plugins\".");

         return (true); // don't allow this pak to get to the server, we handle it
      }

      return (false); // forward this pak to the server
   }

   return (false); // nothing interesting for us, don't drop this pak
}


void Client_ListenToServer (client_t *client)
{
   // this function checks for data from the server directed to client. It polls the loopback
   // to check if data from the server has arrived, and if so, treats it and pass it to the client

   static datagram_t client_datagram; // declared static to speedup recurrent calls of function

   int index;
   bool drop_pak;
   pak_t *pak;
   plugin_t *plugin;
   int plugin_count;

   // get the incoming packet from the server on the client port
   if (!Network_ReceiveAtSocket (&client_datagram, client->socket_client))
      return; // return if nothing's pending

   // we got a packet, allocate space for it
   pak = Pak_New (0, PAKFLAG_SERVER_PAK | PAKFLAG_CRYPTO_VIRCOM);

   // remove Vircom's encryption and get the data
   if (!BuildPakFromUDPDatagrams (&client_datagram, PAKCRYPTO_VERSION_125, pak))
   {
      Pak_Destroy (pak); // skip this packet if we're told to
      return; // this is a fragmented pak and it's not fully arrived yet
   }

   // cycle through all plugins and notify them that the server is sending a pak to this client
   drop_pak = Server_InterceptComms (client, pak); // do a pre-processing of the pak by the proxy
   plugin_count = Plugin_Count (); // get the plugin count and cycle through all of them
   for (index = 0; index < plugin_count; index++)
   {
      plugin = Plugin_GetByIndex (index); // quick access to plugin

      if (!Plugin_IsValid (plugin))
         continue; // skip invalid plugin slots

      // call the pak interception function in the plugin
      if (plugin->PluginInterceptedPakFromServer (client, pak))
         drop_pak = true; // drop this pak if the plugin tells us to
   }

   // are we told to drop this pak ?
   if (drop_pak)
   {
      // we are told to drop this pak

      // was this pak carrying an acknowledgment request ? if so, build the ack replies
      if (pak->flags & PAKFLAG_ACKREQUESTED)
         BuildAckReplyDatagramsFromPak (pak, PAKCRYPTO_VERSION_125, client->socket_client, &server.service_server);

      Pak_Destroy (pak); // enough pak processing
      return; // drop this pak and go and listen to another client
   }

   // rebuild genuine-looking UDP packets from this pak and send them over the wire
   if (client->uses_encryption)
       pak->flags |= PAKFLAG_CRYPTO_XTEA;
   BuildUDPDatagramsFromPak (pak, PAKCRYPTO_VERSION_125, server.socket_fakeserver, &client->service_fakeclient);

   Pak_Destroy (pak); // enough pak processing
   return; // go and listen to another client
}


void Client_SendPakToServer (client_t *client, pak_t *pak)
{
   // this function fakes the specified client sending a pre-built pak to the server

   // rebuild genuine-looking UDP packets from this pak into a new datagram list
   if (pak->flags & PAKFLAG_ACKREQUESTED)
      pak->flags = PAKFLAG_CLIENT_PAK | PAKFLAG_NEW_PAK | PAKFLAG_CRYPTO_VIRCOM | PAKFLAG_ACKREQUESTED;
   else
      pak->flags = PAKFLAG_CLIENT_PAK | PAKFLAG_NEW_PAK | PAKFLAG_CRYPTO_VIRCOM;
   BuildUDPDatagramsFromPak (pak, PAKCRYPTO_VERSION_125, client->socket_client, &server.service_server);

   return; // finished
}
