// network.c

#include "client.h"


// Windows isn't fully POSIX compliant
#ifdef WIN32
#define close(a) closesocket (a)
#endif


// handy defines
#define SOCKET_ALL INVALID_SOCKET


// global variables for internal use only
static unsigned long const_one = 1;
static int const_sockaddr_size = sizeof (struct sockaddr_in);


bool Network_Init (void)
{
   // this function initializes the server's network layer

#ifdef WIN32
   WSADATA wsaData;

   // initialize WinSock
   if (WSAStartup (MAKEWORD (2, 2), &wsaData) != 0)
   {
      Log_Text (LOG_MAIN, "Network_Init(): WinSock initialization failure: %s\n", Util_GetLastNetworkError ());
      return (false); // winsock DLL attaching failed
   }
#endif

   // build the real server service parameters
   SAFE_strncpy (service_server.ip, server_ip, 16);
   service_server.port = atoi (server_port);

   // reset the list of datagrams to be sent
   datagrams = NULL;
   datagram_count = 0;

   PakMachine_Init (); // initialize the pak machine

   Log_Text (LOG_MAIN, "Network_Init(): Network layer successfully initialized.\n");
   return (true); // network successfully initialized
}


void Network_Shutdown (void)
{
   // this function shuts down the server's network layer

   // destroy the list of datagrams to be sent
   SAFE_free (&datagrams); // free the datagrams' allocated memory space
   datagram_count = 0; // and reset their count to zero

   PakMachine_Shutdown (); // shutdown the pak machine

#ifdef WIN32
   WSACleanup (); // clean up Winsock
#endif

   return; // and return
}


void Network_SendAll (void)
{
   // this function is just a high level wrapper for Network_SendFromSocket(), sending
   // all the datagrams in the queue no matter which socket it is from.

   Network_SendFromSocket (SOCKET_ALL); // send the stuff for all the sockets
   return;
}


void Network_SendFromSocket (int socket_from)
{
   // this function sends a datagram list over the network

   int index;
   int error;
   datagram_t *datagram;
   static struct sockaddr_in inet_peer;

   // for each of the datagrams, put them on the network
   for (index = 0; index < datagram_count; index++)
   {
      datagram = &datagrams[index]; // quick access to datagram

      if ((socket_from != SOCKET_ALL) && (socket_from != datagram->our_socket))
         continue; // if socket_from is specified, skip datagrams that don't concern it

      if (!datagram->is_pending || (datagram->send_date > current_time))
         continue; // skip the ones that we've already sent or for which it isn't time yet

      // build the peer service address, internet-style
      inet_peer.sin_family = AF_INET;
      inet_peer.sin_addr.s_addr = inet_addr (datagram->peer_address.ip);
      inet_peer.sin_port = htons ((unsigned short) datagram->peer_address.port);

      // send the datagram over the wire and check for possible failure
      if (sendto (datagram->our_socket, datagram->bytes, datagram->size, 0, (struct sockaddr *) &inet_peer, sizeof (struct sockaddr_in)) == SOCKET_ERROR)
      {
         error = WSAGetLastError (); // ask Windows why it failed

         // did it fail because the send buffer was full ?
         if ((error == WSAENOBUFS) || (error == WSAEWOULDBLOCK))
            datagram->send_date = current_time + 0.1f; // send it again in 0.1 sec

         // else it must have failed for some unknown reason
         else
         {
            Log_Text (LOG_MAIN, "Network_Send(): sendto() failure: %s\n", Util_GetLastNetworkError ());
            datagram->is_pending = false; // datagram could not be sent, forget it
         }
      }

      // else (i.e, no error)
      else
      {
         // log the datagram if server log level is high enough
         if (log_level > 10)
            Log_Datagram (LOG_MAIN, datagram, "Network_Send(): sent datagram to %s:%d\n", datagram->peer_address.ip, datagram->peer_address.port);

         // was this datagram a reliable one ?
         if (datagram->is_reliable)
         {
            datagram->send_date = current_time + 3.0f; // send it again in 3 sec
            datagram->tries++; // this datagram was sent once more

            // is it the tenth time we're trying ?
            if (datagram->tries > 9)
            {
               if (log_level > 1)
                  Log_Datagram (LOG_MAIN, datagram, "Network_Send(): reliable datagram to %s:%d DROPPED (no response from peer)\n", datagram->peer_address.ip, datagram->peer_address.port);
               datagram->is_pending = false; // if it's been sent ten times, forget it
            }
         }
         else
            datagram->is_pending = false; // unreliable datagram has been sent, forget it
      }
   }

   return; // finished, all datagrams sent & list emptied
}


bool Network_ReceiveAtSocket (datagram_t *datagram, int socket_at)
{
   // this function checks if a datagram has arrived from the network and returns true if so

   static struct sockaddr_in sender_address;

   // see what's in the "mailbox"
   datagram->size = recvfrom (socket_at, datagram->bytes, sizeof (datagram->bytes), 0, (struct sockaddr *) &sender_address, &const_sockaddr_size);

   // got NO packet ?
   if (datagram->size == SOCKET_ERROR)
      return (false); // okay, no time to lose

   // we got something, so attribute this datagram the socket at which we received it and
   // copy the sender's address in it
   datagram->our_socket = socket_at;
   SAFE_strncpy (datagram->peer_address.ip, inet_ntoa (sender_address.sin_addr), 16);
   datagram->peer_address.port = ntohs (sender_address.sin_port);

   // is this datagram too short ? (i.e, even a header doesn't fit)
   if (datagram->size < 2)
   {
      Log_Datagram (LOG_MAIN, datagram, "Network_ReceiveAtSocket(): received a datagram too short (%d bytes) from %s:%d, dropping.\n",  datagram->size, datagram->peer_address.ip, datagram->peer_address.port);
      return (false); // this is a bogus packet, drop it
   }

   // else is that datagram too big ?
   else if (datagram->size > 1500)
   {
      Log_Datagram (LOG_MAIN, datagram, "Network_ReceiveAtSocket(): received a datagram too big (%d bytes) from %s:%d, dropping.\n", datagram->size, datagram->peer_address.ip, datagram->peer_address.port);
      return (false); // this is a bogus packet, drop it
   }

   // log the datagram we received if server log level is high enough
   if (log_level > 10)
      Log_Datagram (LOG_MAIN, datagram, "Network_ReceiveAtSocket(): received datagram from %s:%d\n", datagram->peer_address.ip, datagram->peer_address.port);

   return (true); // we got a datagram
}


void Network_PrepareDatagram (datagram_t *datagram, bool is_reliable, int associated_id)
{
   // this function resizes a datagram list to hold a given additional datagram

   int index;
   datagram_t *datagram_slot;

   // cycle through all datagrams in list
   for (index = 0; index < datagram_count; index++)
      if (!datagrams[index].is_pending)
         break; // break as soon as we find a free slot

   // have we reached the end of the datagram list ? (i.e, no free slot found)
   if (index == datagram_count)
   {
      // if so, reallocate the datagram list
      datagrams = (datagram_t *) SAFE_realloc (datagrams, datagram_count, datagram_count + 1, sizeof (datagram_t), false);
      datagram_count++; // the list has now one datagram more
   }

   datagram_slot = &datagrams[index]; // quick access to datagram

   // now copy the datagram in the datagram list (copy just the right amount of bytes)
   datagram_slot->size = datagram->size;
   memcpy (datagram_slot->bytes, datagram->bytes, datagram->size);

   // also copy datagram extra data: source socket and destination address
   datagram_slot->our_socket = datagram->our_socket;
   memcpy (&datagram_slot->peer_address, &datagram->peer_address, sizeof (address_t));

   // date at which it should be sent, whether it's reliable and how many times it's been sent
   datagram_slot->send_date = current_time;
   datagram_slot->is_reliable = is_reliable;
   datagram_slot->associated_id = associated_id;
   datagram_slot->tries = 0;

   datagram_slot->is_pending = true; // there's one datagram more pending
   return; // finished
}


int SenderReceiver_New (int listenport)
{
   // this function creates a sending socket, sets it to non-blocking mode, sets it to reuse
   // bound addresses and binds it to the specified listening port.

   struct sockaddr_in service;
   int out_socket;

   // set this service parameters
   service.sin_family = AF_INET;
   service.sin_addr.s_addr = inet_addr ("0.0.0.0");
   service.sin_port = htons ((unsigned short) listenport);

   // create an UDP socket
   if ((out_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
   {
      Log_Text (LOG_MAIN, "SenderReceiver_New(): Failed creating new socket: %s\n", Util_GetLastNetworkError ());
      return (INVALID_SOCKET);
   }

   // set the socket to non-blocking mode
   if (ioctl (out_socket, FIONBIO, &const_one) == SOCKET_ERROR)
   {
      Log_Text (LOG_MAIN, "SenderReceiver_New(): Failed setting socket to non-blocking mode: %s\n", Util_GetLastNetworkError ());
      close (out_socket); // on error, cleanup and return
      return (INVALID_SOCKET);
   }

   // set the socket to reuse any already bound address/port
   if (setsockopt (out_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &const_one, sizeof (const_one)) == SOCKET_ERROR)
   {
      Log_Text (LOG_MAIN, "SenderReceiver_New(): Failed setting socket to reuse addresses: %s\n", Util_GetLastNetworkError ());
      close (out_socket); // on error, cleanup and return
      return (INVALID_SOCKET);
   }

   // and finally, bind the socket to a listening port
   if (bind (out_socket, (struct sockaddr *) &service, sizeof (struct sockaddr_in)))
   {
      Log_Text (LOG_MAIN, "SenderReceiver_New(): Failed binding new socket to its listening port: %s\n", Util_GetLastNetworkError ());
      close (out_socket); // on error, cleanup and return
      return (INVALID_SOCKET);
   }

   return (out_socket); // network socket created, return it
}


void SenderReceiver_Destroy (int *senderreceiver_socket)
{
   // this function destroys a bound socket, cancelling all unsent datagrams that use that socket

   int index;

   // for starters, send all the datagrams there are to send that use this socket
   Network_SendFromSocket (*senderreceiver_socket);

   // cycle through all datagrams in queue
   for (index = 0; index < datagram_count; index++)
      if (datagrams[index].our_socket == *senderreceiver_socket)
         datagrams[index].is_pending = false; // drop all datagrams using that socket

   // I know, 0 *CAN* be a valid socket number, but it's so convenient!
   if ((*senderreceiver_socket != 0) && (*senderreceiver_socket != INVALID_SOCKET))
      close (*senderreceiver_socket); // close the socket
   *senderreceiver_socket = INVALID_SOCKET;

   return; // finished
}
