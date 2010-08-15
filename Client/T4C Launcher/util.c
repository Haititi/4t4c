// util.c

#include "client.h"


// internal typedefs
typedef struct wsaerror_s
{
   int number;
   const char *description;
} wsaerror_t;


// global variables for internal use only
static wsaerror_t wsa_errors[] =
{
   {6, "WSA_INVALID_HANDLE: Specified event object handle is invalid. [An application attempts to use an event object, but the specified handle is not valid. Note that this error is returned by the operating system, so the error number may change in future releases of Windows.]"},
   {8, "WSA_NOT_ENOUGH_MEMORY: Insufficient memory available. [An application used a Windows Sockets function that directly maps to a Windows function. The Windows function is indicating a lack of required memory resources. Note that this error is returned by the operating system, so the error number may change in future releases of Windows.]"},
   {87, "WSA_INVALID_PARAMETER: One or more parameters are invalid. [An application used a Windows Sockets function which directly maps to a Windows function. The Windows function is indicating a problem with one or more parameters. Note that this error is returned by the operating system, so the error number may change in future releases of Windows.]"},
   {995, "WSA_OPERATION_ABORTED: Overlapped operation aborted. [An overlapped operation was canceled due to the closure of the socket, or the execution of the SIO_FLUSH command in WSAIoctl. Note that this error is returned by the operating system, so the error number may change in future releases of Windows.]"},
   {996, "WSA_IO_INCOMPLETE: Overlapped I/O event object not in signaled state. [The application has tried to determine the status of an overlapped operation which is not yet completed. Applications that use WSAGetOverlappedResult (with the fWait flag set to FALSE) in a polling mode to determine when an overlapped operation has completed, get this error code until the operation is complete. Note that this error is returned by the operating system, so the error number may change in future releases of Windows.]"},
   {997, "WSA_IO_PENDING: Overlapped operations will complete later. [The application has initiated an overlapped operation that cannot be completed immediately. A completion indication will be given later when the operation has been completed. Note that this error is returned by the operating system, so the error number may change in future releases of Windows.]"},
   {10004, "WSAEINTR: Interrupted function call. [A blocking operation was interrupted by a call to WSACancelBlockingCall.]"},
   {10009, "WSAEBADF: File handle is not valid. [The file handle supplied is not valid.]"},
   {10013, "WSAEACCES: Permission denied. [An attempt was made to access a socket in a way forbidden by its access permissions. An example is using a broadcast address for sendto without broadcast permission being set using setsockopt(SO_BROADCAST). Another possible reason for the WSAEACCES error is that when the bind function is called (on Windows NT 4 SP4 or later), another application, service, or kernel mode driver is bound to the same address with exclusive access. Such exclusive access is a new feature of Windows NT 4 SP4 and later, and is implemented by using the SO_EXCLUSIVEADDRUSE option.]"},
   {10014, "WSAEFAULT: Bad address. [The system detected an invalid pointer address in attempting to use a pointer argument of a call. This error occurs if an application passes an invalid pointer value, or if the length of the buffer is too small. For instance, if the length of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr).]"},
   {10022, "WSAEINVAL: Invalid argument. [Some invalid argument was supplied (for example, specifying an invalid level to the setsockopt function). In some instances, it also refers to the current state of the socket—for instance, calling accept on a socket that is not listening.]"},
   {10024, "WSAEMFILE: Too many open files. [Too many open sockets. Each implementation may have a maximum number of socket handles available, either globally, per process, or per thread.]"},
   {10035, "WSAEWOULDBLOCK: Resource temporarily unavailable. [This error is returned from operations on nonblocking sockets that cannot be completed immediately, for example recv when no data is queued to be read from the socket. It is a nonfatal error, and the operation should be retried later. It is normal for WSAEWOULDBLOCK to be reported as the result from calling connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the connection to be established.]"},
   {10036, "WSAEINPROGRESS: Operation now in progress. [A blocking operation is currently executing. Windows Sockets only allows a single blocking operation—per- task or thread—to be outstanding, and if any other function call is made (whether or not it references that or any other socket) the function fails with the WSAEINPROGRESS error.]"},
   {10037, "WSAEALREADY: Operation already in progress. [An operation was attempted on a nonblocking socket with an operation already in progress—that is, calling connect a second time on a nonblocking socket that is already connecting, or canceling an asynchronous request (WSAAsyncGetXbyY) that has already been canceled or completed.]"},
   {10038, "WSAENOTSOCK: Socket operation on nonsocket. [An operation was attempted on something that is not a socket. Either the socket handle parameter did not reference a valid socket, or for select, a member of an fd_set was not valid.]"},
   {10039, "WSAEDESTADDRREQ: Destination address required. [A required address was omitted from an operation on a socket. For example, this error is returned if sendto is called with the remote address of ADDR_ANY.]"},
   {10040, "WSAEMSGSIZE: Message too long. [A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram was smaller than the datagram itself.]"},
   {10041, "WSAEPROTOTYPE: Protocol wrong type for socket. [A protocol was specified in the socket function call that does not support the semantics of the socket type requested. For example, the ARPA Internet UDP protocol cannot be specified with a socket type of SOCK_STREAM.]"},
   {10042, "WSAENOPROTOOPT: Bad protocol option. [An unknown, invalid or unsupported option or level was specified in a getsockopt or setsockopt call.]"},
   {10043, "WSAEPROTONOSUPPORT: Protocol not supported. [The requested protocol has not been configured into the system, or no implementation for it exists. For example, a socket call requests a SOCK_DGRAM socket, but specifies a stream protocol.]"},
   {10044, "WSAESOCKTNOSUPPORT: Socket type not supported. [The support for the specified socket type does not exist in this address family. For example, the optional type SOCK_RAW might be selected in a socket call, and the implementation does not support SOCK_RAW sockets at all.]"},
   {10045, "WSAEOPNOTSUPP: Operation not supported. [The attempted operation is not supported for the type of object referenced. Usually this occurs when a socket descriptor to a socket that cannot support this operation is trying to accept a connection on a datagram socket.]"},
   {10046, "WSAEPFNOSUPPORT: Protocol family not supported. [The protocol family has not been configured into the system or no implementation for it exists. This message has a slightly different meaning from WSAEAFNOSUPPORT. However, it is interchangeable in most cases, and all Windows Sockets functions that return one of these messages also specify WSAEAFNOSUPPORT.]"},
   {10047, "WSAEAFNOSUPPORT: Address family not supported by protocol family. [An address incompatible with the requested protocol was used. All sockets are created with an associated address family (that is, AF_INET for Internet Protocols) and a generic protocol type (that is, SOCK_STREAM). This error is returned if an incorrect protocol is explicitly requested in the socket call, or if an address of the wrong family is used for a socket, for example, in sendto.]"},
   {10048, "WSAEADDRINUSE: Address already in use. [Typically, only one usage of each socket address (protocol/IP address/port) is permitted. This error occurs if an application attempts to bind a socket to an IP address/port that has already been used for an existing socket, or a socket that was not closed properly, or one that is still in the process of closing. For server applications that need to bind multiple sockets to the same port number, consider using setsockopt (SO_REUSEADDR). Client applications usually need not call bind at all— connect chooses an unused port automatically. When bind is called with a wildcard address (involving ADDR_ANY), a WSAEADDRINUSE error could be delayed until the specific address is committed. This could happen with a call to another function later, including connect, listen, WSAConnect, or WSAJoinLeaf.]"},
   {10049, "WSAEADDRNOTAVAIL: Cannot assign requested address. [The requested address is not valid in its context. This normally results from an attempt to bind to an address that is not valid for the local computer. This can also result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote address or port is not valid for a remote computer (for example, address or port 0).]"},
   {10050, "WSAENETDOWN: Network is down. [A socket operation encountered a dead network. This could indicate a serious failure of the network system (that is, the protocol stack that the Windows Sockets DLL runs over), the network interface, or the local network itself.]"},
   {10051, "WSAENETUNREACH: Network is unreachable. [A socket operation was attempted to an unreachable network. This usually means the local software knows no route to reach the remote host.]"},
   {10052, "WSAENETRESET: Network dropped connection on reset. [The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress. It can also be returned by setsockopt if an attempt is made to set SO_KEEPALIVE on a connection that has already failed.]"},
   {10053, "WSAECONNABORTED: Software caused connection abort. [An established connection was aborted by the software in your host computer, possibly due to a data transmission time-out or protocol error.]"},
   {10054, "WSAECONNRESET: Connection reset by peer. [An existing connection was forcibly closed by the remote host. This normally results if the peer application on the remote host is suddenly stopped, the host is rebooted, the host or remote network interface is disabled, or the remote host uses a hard close (see setsockopt for more information on the SO_LINGER option on the remote socket). This error may also result if a connection was broken due to keep-alive activity detecting a failure while one or more operations are in progress. Operations that were in progress fail with WSAENETRESET. Subsequent operations fail with WSAECONNRESET.]"},
   {10055, "WSAENOBUFS: No buffer space available. [An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.]"},
   {10056, "WSAEISCONN: Socket is already connected. [A connect request was made on an already-connected socket. Some implementations also return this error if sendto is called on a connected SOCK_DGRAM socket (for SOCK_STREAM sockets, the to parameter in sendto is ignored) although other implementations treat this as a legal occurrence.]"},
   {10057, "WSAENOTCONN: Socket is not connected. [A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using sendto) no address was supplied. Any other type of operation might also return this error—for example, setsockopt setting SO_KEEPALIVE if the connection has been reset.]"},
   {10058, "WSAESHUTDOWN: Cannot send after socket shutdown. [A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call. By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued.]"},
   {10059, "WSAETOOMANYREFS: Too many references. [Too many references to some kernel object.]"},
   {10060, "WSAETIMEDOUT: Connection timed out. [A connection attempt failed because the connected party did not properly respond after a period of time, or the established connection failed because the connected host has failed to respond.]"},
   {10061, "WSAECONNREFUSED: Connection refused. [No connection could be made because the target computer actively refused it. This usually results from trying to connect to a service that is inactive on the foreign host—that is, one with no server application running.]"},
   {10062, "WSAELOOP: Cannot translate name. [Cannot translate a name.]"},
   {10063, "WSAENAMETOOLONG: Name too long. [A name component or a name was too long.]"},
   {10064, "WSAEHOSTDOWN: Host is down. [A socket operation failed because the destination host is down. A socket operation encountered a dead host. Networking activity on the local host has not been initiated. These conditions are more likely to be indicated by the error WSAETIMEDOUT.]"},
   {10065, "WSAEHOSTUNREACH: No route to host. [A socket operation was attempted to an unreachable host. See WSAENETUNREACH.]"},
   {10066, "WSAENOTEMPTY: Directory not empty. [Cannot remove a directory that is not empty.]"},
   {10067, "WSAEPROCLIM: Too many processes. [A Windows Sockets implementation may have a limit on the number of applications that can use it simultaneously.WSAStartup may fail with this error if the limit has been reached.]"},
   {10068, "WSAEUSERS: User quota exceeded. [Ran out of user quota.]"},
   {10069, "WSAEDQUOT: Disk quota exceeded. [Ran out of disk quota.]"},
   {10070, "WSAESTALE: Stale file handle reference. [The file handle reference is no longer available.]"},
   {10071, "WSAEREMOTE: Item is remote. [The item is not available locally.]"},
   {10091, "WSASYSNOTREADY: Network subsystem is unavailable. [This error is returned by WSAStartup if the Windows Sockets implementation cannot function at this time because the underlying system it uses to provide network services is currently unavailable. Users should check that the appropriate Windows Sockets DLL file is in the current path, that they are not trying to use more than one Windows Sockets implementation simultaneously. If there is more than one Winsock DLL on your system, be sure the first one in the path is appropriate for the network subsystem currently loaded, that the Windows Sockets implementation documentation to be sure all necessary components are currently installed and configured correctly.]"},
   {10092, "WSAVERNOTSUPPORTED: Winsock.dll version out of range. [The current Windows Sockets implementation does not support the Windows Sockets specification version requested by the application. Check that no old Windows Sockets DLL files are being accessed.]"},
   {10093, "WSANOTINITIALISED: Successful WSAStartup not yet performed. [Either the application has not called WSAStartup or WSAStartup failed. The application may be accessing a socket that the current active task does not own (that is, trying to share a socket between tasks), or WSACleanup has been called too many times.]"},
   {10101, "WSAEDISCON: Graceful shutdown in progress. [Returned by WSARecv and WSARecvFrom to indicate that the remote party has initiated a graceful shutdown sequence.]"},
   {10102, "WSAENOMORE: No more results. [No more results can be returned by the WSALookupServiceNext function.]"},
   {10103, "WSAECANCELLED: Call has been canceled. [A call to the WSALookupServiceEnd function was made while this call was still processing. The call has been canceled.]"},
   {10104, "WSAEINVALIDPROCTABLE: Procedure call table is invalid. [The service provider procedure call table is invalid. A service provider returned a bogus procedure table to Ws2_32.dll. This is usually caused by one or more of the function pointers being NULL.]"},
   {10105, "WSAEINVALIDPROVIDER: Service provider is invalid. [The requested service provider is invalid. This error is returned by the WSCGetProviderInfo and WSCGetProviderInfo32 functions if the protocol entry specified could not be found. This error is also returned if the service provider returned a version number other than 2.0.]"},
   {10106, "WSAEPROVIDERFAILEDINIT: Service provider failed to initialize. [The requested service provider could not be loaded or initialized. This error is returned if either a service provider's DLL could not be loaded (LoadLibrary failed) or the provider's WSPStartup or NSPStartup function failed.]"},
   {10107, "WSASYSCALLFAILURE: System call failure. [A system call that should never fail has failed. This is a generic error code, returned under various condition. Returned when a system call that should never fail does fail. For example, if a call to WaitForMultipleEvents fails or one of the registry functions fails trying to manipulate the protocol/namespace catalogs. Returned when a provider does not return SUCCESS and does not provide an extended error code. Can indicate a service provider implementation error.]"},
   {10108, "WSASERVICE_NOT_FOUND: Service not found. [No such service is known. The service cannot be found in the specified name space.]"},
   {10109, "WSATYPE_NOT_FOUND: Class type not found. [The specified class was not found.]"},
   {10110, "WSA_E_NO_MORE: No more results. [No more results can be returned by the WSALookupServiceNext function.]"},
   {10111, "WSA_E_CANCELLED: Call was canceled. [A call to the WSALookupServiceEnd function was made while this call was still processing. The call has been canceled.]"},
   {10112, "WSAEREFUSED: Database query was refused. [A database query failed because it was actively refused.]"},
   {11001, "WSAHOST_NOT_FOUND: Host not found. [No such host is known. The name is not an official host name or alias, or it cannot be found in the database(s) being queried. This error may also be returned for protocol and service queries, and means that the specified name could not be found in the relevant database.]"},
   {11002, "WSATRY_AGAIN: Nonauthoritative host not found. [This is usually a temporary error during host name resolution and means that the local server did not receive a response from an authoritative server. A retry at some time later may be successful.]"},
   {11003, "WSANO_RECOVERY: This is a nonrecoverable error. [This indicates that some sort of nonrecoverable error occurred during a database lookup. This may be because the database files (for example, BSD-compatible HOSTS, SERVICES, or PROTOCOLS files) could not be found, or a DNS request was returned by the server with a severe error.]"},
   {11004, "WSANO_DATA: Valid name, no data record of requested type. [The requested name is valid and was found in the database, but it does not have the correct associated data being resolved for. The usual example for this is a host name-to-address translation attempt (using gethostbyname or WSAAsyncGetHostByName) which uses the DNS (Domain Name Server). An MX record is returned but no A record—indicating the host itself exists, but is not directly reachable.]"},
   {11005, "WSA_QOS_RECEIVERS: QOS receivers. [At least one QOS reserve has arrived.]"},
   {11006, "WSA_QOS_SENDERS: QOS senders. [At least one QOS send path has arrived.]"},
   {11007, "WSA_QOS_NO_SENDERS: No QOS senders. [There are no QOS senders.]"},
   {11008, "WSA_QOS_NO_RECEIVERS: QOS no receivers. [There are no QOS receivers.]"},
   {11009, "WSA_QOS_REQUEST_CONFIRMED: QOS request confirmed. [The QOS reserve request has been confirmed.]"},
   {11010, "WSA_QOS_ADMISSION_FAILURE: QOS admission error. [A QOS error occurred due to lack of resources.]"},
   {11011, "WSA_QOS_POLICY_FAILURE: QOS policy failure. [The QOS request was rejected because the policy system couldn't allocate the requested resource within the existing policy.]"},
   {11012, "WSA_QOS_BAD_STYLE: QOS bad style. [An unknown or conflicting QOS style was encountered.]"},
   {11013, "WSA_QOS_BAD_OBJECT: QOS bad object. [A problem was encountered with some part of the filterspec or the provider-specific buffer in general.]"},
   {11014, "WSA_QOS_TRAFFIC_CTRL_ERROR: QOS traffic control error. [An error with the underlying traffic control (TC) API as the generic QOS request was converted for local enforcement by the TC API. This could be due to an out of memory error or to an internal QOS provider error.]"},
   {11015, "WSA_QOS_GENERIC_ERROR: QOS generic error. [A general QOS error.]"},
   {11016, "WSA_QOS_ESERVICETYPE: QOS service type error. [An invalid or unrecognized service type was found in the QOS flowspec.]"},
   {11017, "WSA_QOS_EFLOWSPEC: QOS flowspec error. [An invalid or inconsistent flowspec was found in the QOS structure.]"},
   {11018, "WSA_QOS_EPROVSPECBUF: Invalid QOS provider buffer. [An invalid QOS provider-specific buffer.]"},
   {11019, "WSA_QOS_EFILTERSTYLE: Invalid QOS filter style. [An invalid QOS filter style was used.]"},
   {11020, "WSA_QOS_EFILTERTYPE: Invalid QOS filter type. [An invalid QOS filter type was used.]"},
   {11021, "WSA_QOS_EFILTERCOUNT: Incorrect QOS filter count. [An incorrect number of QOS FILTERSPECs were specified in the FLOWDESCRIPTOR.]"},
   {11022, "WSA_QOS_EOBJLENGTH: Invalid QOS object length. [An object with an invalid ObjectLength field was specified in the QOS provider-specific buffer.]"},
   {11023, "WSA_QOS_EFLOWCOUNT: Incorrect QOS flow count. [An incorrect number of flow descriptors was specified in the QOS structure.]"},
   {11024, "WSA_QOS_EUNKOWNPSOBJ: Unrecognized QOS object. [An unrecognized object was found in the QOS provider-specific buffer.]"},
   {11025, "WSA_QOS_EPOLICYOBJ: Invalid QOS policy object. [An invalid policy object was found in the QOS provider-specific buffer.]"},
   {11026, "WSA_QOS_EFLOWDESC: Invalid QOS flow descriptor. [An invalid QOS flow descriptor was found in the flow descriptor list.]"},
   {11027, "WSA_QOS_EPSFLOWSPEC: Invalid QOS provider-specific flowspec. [An invalid or inconsistent flowspec was found in the QOS provider-specific buffer.]"},
   {11028, "WSA_QOS_EPSFILTERSPEC: Invalid QOS provider-specific filterspec. [An invalid FILTERSPEC was found in the QOS provider-specific buffer.]"},
   {11029, "WSA_QOS_ESDMODEOBJ: Invalid QOS shape discard mode object. [An invalid shape discard mode object was found in the QOS provider-specific buffer.]"},
   {11030, "WSA_QOS_ESHAPERATEOBJ: Invalid QOS shaping rate object. [An invalid shaping rate object was found in the QOS provider-specific buffer.]"},
   {11031, "WSA_QOS_RESERVED_PETYPE: Reserved policy QOS element type. [A reserved policy element was found in the QOS provider-specific buffer.]"}
};


const char *Util_GetLastError (void)
{
   // this function retrieves and translates the last system error code into a full text string

   static char error_buffer[1024];
   int length;
   int error;

   error = GetLastError (); // first get the error code from the system

   // format it and ask the system for the description
   SAFE_snprintf (error_buffer, sizeof (error_buffer), "%05d: ", error);
   length = strlen (error_buffer);
   FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, error_buffer + length, sizeof (error_buffer) - length, NULL);

   // and return a pointer to it
   return (error_buffer);
}


const char *Util_GetLastNetworkError (void)
{
   // this function retrieves and translates the last WSA error code into a full text string

   size_t i;
   int error;

   error = WSAGetLastError (); // first get the error code from the system

   // for each error code we know, see if it's the one we want
   for (i = 0; i < sizeof (wsa_errors) / sizeof (wsaerror_t); i++)
      if (wsa_errors[i].number == error)
         return (wsa_errors[i].description); // if it's that one, return its description

   // error code not found
   return ("Unknown error.");
}


void Util_MakeAbsolutePathname (const char *pathname, char *absolute_pathname, int max_absolute_pathname_length)
{
   // this function ensures pathname is an absolute path. If it isn't, prepend the game directory
   // path to it so as to ensure it will be one. System services require absolute paths for file
   // I/O, because their working directory is the system directory.

   int length;
   int i;

   // absolute or relative pathname ?
   if ((pathname[0] == '\\') || (pathname[0] == '/') || (strstr (pathname, ":") != NULL))
      SAFE_strncpy (absolute_pathname, pathname, max_absolute_pathname_length);
   else
      SAFE_snprintf (absolute_pathname, max_absolute_pathname_length, "%s/%s", app_path, pathname);

   // convert all slashes to system-specific ones
   length = strlen (absolute_pathname);
   for (i = 0; i < length; i++)
#ifdef WIN32
      if (absolute_pathname[i] == '/')
         absolute_pathname[i] = '\\';
#else
      if (absolute_pathname[i] == '\\')
         absolute_pathname[i] = '/';
#endif

   return; // finished
}


unsigned long Util_LookupAddress (const char *address_as_string)
{
   // this function resolves a name to an IP address and returns the result as an unsigned long (4 bytes)

   unsigned long address;
   struct hostent *host;

   // try looking up as a raw address first
   address = inet_addr (address_as_string);
   if (address != INADDR_NONE)
      return (address);

   // if it fails (isn't a dotted IP), resolve it through DNS
   host = gethostbyname (address_as_string);
   if (host != 0)
      return (*((unsigned long *) host->h_addr_list[0]));

   // address didn't resolve
   return (INADDR_NONE);
}


const char *Util_GetDirectoryPath (const char *pathname, char *path)
{
   // this function builds a directory path out of a full file pathname

   int index;
   int length;

   length = (int) strlen (pathname); // get length of pathname first
   if (length > MAX_STRING8 - 1)
      length = MAX_STRING8 - 1; // bound it to MAX_STRING8 characters max

   for (index = 0; index < length; index++)
   {
      path[index] = pathname[index]; // now copy pathname in the destination string
      if (pathname[index] == 0)
         break; // don't copy beyond the end of source
   }
   path[length] = 0; // terminate the string

   // now scan the destination string starting from the end until a field separator is found
   while ((length > 0) && !((path[length] == '\\') || (path[length] == '/')) && (path[length] != ':'))
      length--; // go back one character after the other as long as it's not the case

   // given the type of field separator we stopped on, keep it or not
   if (path[length] == ':')
      length++; // if it's a disk letter separator, keep it

   path[length] = 0; // terminate the string at this position
   return (path); // and return a pointer to it
}


float ProcessTime (void)
{
   // this function returns the time in seconds elapsed since the executable process started.
   // The rollover check ensures the program will continue running after clock() will have
   // overflown its integer value (it does so every 24 days or so). With this rollover check
   // we have a lifetime of more than billion years, w00t!
   // thanks to botmeister for the rollover check idea.

   static long prev_clock = 0;
   static long rollover_count = 0;
   long current_clock;
   double time_in_seconds;

   current_clock = clock (); // get system clock

   // has the clock overflown ?
   if (current_clock < prev_clock)
      rollover_count++; // omg, it has, we're running for more than 24 days!

   // now convert the time to seconds since last rollover
   time_in_seconds = (double) current_clock / CLOCKS_PER_SEC; // convert clock to seconds

   prev_clock = current_clock; // keep track of current time for future calls of this function

   // and return the time in seconds, adding the overflow differences if necessary.
   // HACK: grant the timer to start at 60 seconds to ensure all timer checks work well
   return ((float) (60.0f + time_in_seconds + (((double) LONG_MAX + 1.0) / CLOCKS_PER_SEC) * rollover_count));
}


bool WindowsShell_CopyDirectory (char *source_pathname, char *target_pathname)
{
   // this function is a wrapper that asks the Windows shell to copy a file or a directory.
   // Since it's done by the shell, it can copy recursively any sort of directory.

   char source[512]; // source path must be double null-terminated, so go beyond MAX_PATH
   char destination[512]; // source path must be double null-terminated, so go beyond MAX_PATH
   int length;
   int index;
   SHFILEOPSTRUCT fileop;

   for (index = 0; index < sizeof (source); index++)
   {
      if (source_pathname[index] != '/')
         source[index] = source_pathname[index]; // copy source path
      else
         source[index] = '\\'; // convert all forward slashes to Windows' fashion

      if (source_pathname[index] == 0)
         break; // don't copy beyond the end of string
   }
   length = strlen (source);
   source[length + 1] = 0; // append a second null terminator

   for (index = 0; index < sizeof (destination); index++)
   {
      if (target_pathname[index] != '/')
         destination[index] = target_pathname[index]; // copy destination path
      else
         destination[index] = '\\'; // convert all forward slashes to Windows' fashion

      if (target_pathname[index] == 0)
         break; // don't copy beyond the end of string
   }
   length = strlen (destination);
   destination[length + 1] = 0; // append a second null terminator

   fileop.hwnd = NULL; // no status display
   fileop.wFunc = FO_COPY; // copy operation
   fileop.pFrom = source; // source file name as double null terminated string
   fileop.pTo = destination; // destination as double null terminated string
   fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT; // do not prompt the user
   fileop.fAnyOperationsAborted = FALSE;
   fileop.lpszProgressTitle = NULL;
   fileop.hNameMappings = NULL;

   return (SHFileOperation (&fileop) == 0); // do the file operation and return results
}


bool WindowsShell_DeleteDirectory (char *pathname)
{
   // this function is a wrapper that asks the Windows shell to delete a file or a directory.
   // Since it's done by the shell, it can delete recursively a non-empty directory.

   char source[512]; // source path must be double null-terminated, so go beyond MAX_PATH
   int length;
   int index;
   SHFILEOPSTRUCT fileop;

   for (index = 0; index < sizeof (source); index++)
   {
      if (pathname[index] != '/')
         source[index] = pathname[index]; // copy source path
      else
         source[index] = '\\'; // convert all forward slashes to Windows' fashion

      if (pathname[index] == 0)
         break; // don't copy beyond the end of string
   }
   length = strlen (source);
   source[length + 1] = 0; // append a second null terminator

   fileop.hwnd = NULL; // no status display
   fileop.wFunc = FO_DELETE; // delete operation
   fileop.pFrom = source; // source file name as double null terminated string
   fileop.pTo = NULL; // no destination needed
   fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT; // do not prompt the user
   fileop.fAnyOperationsAborted = FALSE;
   fileop.lpszProgressTitle = NULL;
   fileop.hNameMappings = NULL;

   return (SHFileOperation (&fileop) == 0); // do the file operation and return results
}


int MessageBox_printf (unsigned long mb_type, char *title, char *fmt, ...)
{
   // this function prints a message in a message box. It enables the caller to use variable
   // length arguments, printf-style, and returns the result of the MessageBox() call.

   va_list argptr;
   char message[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   SAFE_vsnprintf (message, sizeof (message), fmt, argptr);
   va_end (argptr);

   // call MessageBox() and return the results
   return (MessageBox (NULL, message, title, mb_type));
}
