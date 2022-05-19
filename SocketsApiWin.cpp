// Copyright Eric Chauvin 2022



// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#include "SocketsApiWin.h"
// #include "SocketsApiLinux.h"
#include "../CppBase/Casting.h"


// I hate to have to put a #define statement
// into any of my code.  But if you need to
// include the Windows.h file then
// you have to define

// #define WIN32_LEAN_AND_MEAN

// So that it doesn't include winsock.h.
// And of course this would only be done in
// a .cpp file and not a header file
// #include <windows.h>

// #include <stdio.h>


// For Windows.
// #include <winsock.h>
#include <WinSock2.h>
#include <WS2tcpip.h> // getaddrinfo()

// Need to link with Ws2_32.lib,
// Mswsock.lib,
// and Advapi32.lib for security and
// registry calls?

// #pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")
// #pragma comment (lib, "AdvApi32.lib")


// For Linux.
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>



SocketsApi::SocketsApi( void )
{
// See BuildProj.bat for how to link to the
// Windows .lib file.

// For Windows.
WSADATA wsaData;

// MAKEWORD(1,1) for Winsock 1.1,
// MAKEWORD(2,0) for Winsock 2.0:
// MAKEWORD(2,2) for Winsock 2.2:

if( WSAStartup( MAKEWORD(2,2), &wsaData ) != 0 )
  throw "WSAStartup didn't work.";

}



SocketsApi::SocketsApi( const SocketsApi& in )
{
// Make the compiler think the in value is
// being used.
if( in.testForCopy == 123 )
  return;

const char* showS = "The SocketsApi copy"
         " constructor should not get called.";

throw showS;
}



SocketsApi::~SocketsApi( void )
{
// For Windows.
WSACleanup();
}


/*
SocketCpp SocketsApi::getInvalidSocket( void )
{
if( InvalidSocket == INVALID_SOCKET )
  return 0;

// This is unreachable code because it is zero.
return INVALID_SOCKET;
}
*/



void SocketsApi::closeSocket( SocketCpp toClose )
{
if( toClose == 0 )
  return;

// For Windows.
// returns zero on no error.

//    iResult = shutdown(ConnectSocket, SD_SEND);

closesocket( toClose );
// Linux uses close();
}



SocketCpp SocketsApi::openClient(
                        const char* domain,
                        const char* port,
                        CharBuf& errorBuf )
{
// result is a linked list.
// In Linux is it a pointer to a pointer?
// const struct addrinfo** result;

struct addrinfo* result = nullptr;
struct addrinfo* ptr = nullptr;
struct addrinfo hints;

// memset( &hints, 0, sizeof( hints ));
ZeroMemory( &hints, sizeof(hints) );

// It's unspecified so it's either IPV4 or IPV6.

hints.ai_family = AF_UNSPEC;

hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;

// Port 443 for https.

// htons() host to network short
// htonl() host to network long
// ntohs() network to host short
// ntohl() network to host long

Int32 status = getaddrinfo(
              domain, // "www.thedomain.com"
              port, // "443" "https", "ftp", etc.
              &hints,
              &result );

if( status != 0 )
  {
  errorBuf.appendChars(
               "SocketsApi getaddrinfo error.\n" );

  // 11001 is host not found.
  Int32 error = WSAGetLastError();
  errorBuf.appendChars(
               "clientConnect() error.\n" );

  if( error == WSAHOST_NOT_FOUND )
    errorBuf.appendChars( "Host not found.\n" );

  errorBuf.appendChars( "Error is: " );
  Str errorS( error );
  errorBuf.appendStr( errorS );
  errorBuf.appendChars( "\n" );

  return 0;
  }

// SOCKET clientSocket = INVALID_SOCKET;
SocketCpp clientSocket = INVALID_SOCKET;

// Try the possible connections.
Int32 count = 0;
for( ptr = result; ptr != nullptr;
                              ptr = ptr->ai_next )
  {
  // Make sure nothing is really-bad wrong here.
  count++;
  if( count > 5 )
    {
    errorBuf.appendChars(
     "SocketWin too many sockets for connect.\n" );

    freeaddrinfo( result );
    return 0;
    }

  clientSocket = socket( ptr->ai_family,
                          ptr->ai_socktype,
                          ptr->ai_protocol );

  if( clientSocket == INVALID_SOCKET )
    {
    errorBuf.appendChars(
      "SocketWin no sockets left for connect.\n" );

    // WSAGetLastError());
    freeaddrinfo( result );
    return 0;
    }

  Int32 connectResult = connect( clientSocket,
                         ptr->ai_addr,
                         Casting::U64ToI32(
                         ptr->ai_addrlen ));

  if( connectResult == SOCKET_ERROR )
    {
    errorBuf.appendChars(
      "SocketWin trying the next socket.\n" );

    closesocket( clientSocket );
    clientSocket = INVALID_SOCKET;
    continue; // Try to connect to the next
              // valid socket.
    }

  // It should have a good connected socket.
  break;
  }

errorBuf.appendChars(
        "SocketsApi connected to " );
errorBuf.appendChars( domain );
errorBuf.appendChars( "\n" );

freeaddrinfo( result );

// Make it non blocking.  0 is blocking.
// Non zero is non blocking.
Uint32L iMode = 1;

// setsockopt()

// for Linux:
// #include <unistd.h>
// #include <fcntl.h>
// sockfd = socket(PF_INET, SOCK_STREAM, 0);
// fcntl(sockfd, F_SETFL, O_NONBLOCK);

// #define FIONBIO     _IOW('f', 126, u_long)
status = ioctlsocket( clientSocket,
                    Casting::u32ToI32ForMacro(
                    FIONBIO ),
                    // FIONBIO,
                    &iMode );
if( status != 0 )
  {
  Int32 error = WSAGetLastError();
  errorBuf.appendChars(
                 "socket ioctl failed.\n" );
  errorBuf.appendChars( "Error is: " );
  Str errorS( error );
  errorBuf.appendStr( errorS );
  errorBuf.appendChars( "\n" );
  return 0;
  }

return clientSocket;
}




SocketCpp SocketsApi::openServer( const char* port,
                               CharBuf& errorBuf )
{
// For the server class...
// Check the stats and hacking info for disallowed
// IP addresses and things.
// Clients connecting from the same NAT address.

// result is a linked list.
// In Linux is it a pointer to a pointer?
// const struct addrinfo** result;

struct addrinfo* result = nullptr;
// struct addrinfo* addrPtr = nullptr;
struct addrinfo hints;

// memset( &hints, 0, sizeof( hints ));
ZeroMemory( &hints, sizeof(hints) );

// It's unspecified so it's either IPV4 or IPV6.

hints.ai_family = AF_UNSPEC;

hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;
hints.ai_flags = AI_PASSIVE; // Get the IP.


// Port 443 for https.

// getaddrinfo is in ws2tcpip.h.

// The domain can be "localhost".
// An empty string means all addresses on the local
// computer are returned.
// On Linux is it nullptr instead of ""?
// Make it the IP address if you wanted to bind
// it to a specific IP address.

Int32 status = getaddrinfo(
              "", // "www.thedomain.com"
              port, // "443" "https", "ftp", etc.
              &hints,
              &result );

if( status != 0 )
  {
  errorBuf.appendChars(
         "SocketWin server getaddrinfo error.\n" );
  return 0;
  }

errorBuf.appendChars(
         "Before opening socket.\n" );

// SOCKET serverSocket = INVALID_SOCKET;
SocketCpp serverSocket = socket(
                          result->ai_family,
                          result->ai_socktype,
                          result->ai_protocol );

if( serverSocket == INVALID_SOCKET )
  {
  errorBuf.appendChars(
    "SocketWin  server no sockets.\n" );

  // WSAGetLastError());
  freeaddrinfo( result );
  return 0;
  }

if( 0 != bind( serverSocket, result->ai_addr,
               Casting::U64ToI32(
               result->ai_addrlen )))
  {
  closeSocket( serverSocket );
  errorBuf.appendChars(
    "SocketWin  server bind error.\n" );

  freeaddrinfo( result );
  return 0;
  }

// The backlog could be several hundred in Windows.
// What is it in Linux?
if( 0 != listen( serverSocket, 20 ))
  {
  closeSocket( serverSocket );
  errorBuf.appendChars(
    "SocketApi  server listen error.\n" );

  freeaddrinfo( result );
  return 0;
  }

errorBuf.appendChars(
        "SocketsApi server got socket.\n" );

freeaddrinfo( result );

// Make it non blocking.  0 is blocking.
// Non zero is non blocking.
Uint32L iMode = 1;

// setsockopt()

// for Linux:
// #include <unistd.h>
// #include <fcntl.h>
// sockfd = socket(PF_INET, SOCK_STREAM, 0);
// fcntl(sockfd, F_SETFL, O_NONBLOCK);

// C:\Program Files (x86)\Windows Kits\
//         10\include\10.0.19041.0\um\winsock2.h

// #define FIONBIO     _IOW('f', 126, u_long)
status = ioctlsocket( serverSocket,
                    Casting::u32ToI32ForMacro(
                    FIONBIO ),
                    // FIONBIO,
                    &iMode );
if( status != 0 )
  {
  Int32 error = WSAGetLastError();
  errorBuf.appendChars(
                 "socket ioctl failed.\n" );
  errorBuf.appendChars( "Error is: " );
  Str errorS( error );
  errorBuf.appendStr( errorS );
  errorBuf.appendChars( "\n" );
  return 0;
  }


//  Int32 error = WSAGetLastError();
// errorBuf.appendChars(
//               "socket ioctl failed.\n" );
//   errorBuf.appendChars( "Error is: " );
//  Str errorS( error );
//  errorBuf.appendStr( errorS );
//  errorBuf.appendChars( "\n" );

return serverSocket;
}



/*
// Also see poll().
bool SocketsWin::checkSelect(
                         SocketCpp servSock,
                         CharBuf& errorBuf )
{
struct timeval tv;
fd_set readfds;

tv.tv_sec = 0;
tv.tv_usec = 500000; // microseconds.

FD_ZERO( &readfds );
FD_SET( servSock, &readfds );

select( Casting::U64ToI32( servSock + 1 ),
                 &readfds, nullptr,
                 nullptr, &tv );

if( FD_ISSET( servSock, &readfds ))
  {
  errorBuf.appendChars(
                "Select says ready to read.\n" );
  return true;
  }

errorBuf.appendChars(
                "Select not ready.\n" );
return false;
}
*/



SocketCpp SocketsApi::acceptConnect(
                         SocketCpp servSock,
                         CharBuf& fromCBuf,
                         CharBuf& errorBuf )
{
//  127.0.0.1

struct sockaddr_storage remoteAddr;
Int32 addrSize = sizeof( remoteAddr );

// if( !checkSelect( servSock, errorBuf ))
  // return 0;

// I hate to have to do stuff like this:
// The new style static_cast won't work here
// either.
// static_cast<>()

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"

SocketCpp acceptSock = accept( servSock,
              (struct sockaddr *)&remoteAddr,
              &addrSize );

#pragma clang diagnostic pop

// This causes the warning after the pop.
// accept( servSock,
//              (struct sockaddr *)&remoteAddr,
//              &addrSize );

if( acceptSock == INVALID_SOCKET )
  {
  // It's non-blocking and there is nothing there.
  // errorBuf.appendChars(
  //            "Accepted socket is invalid.\n" );
  return INVALID_SOCKET;
  }



#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"

// IPv4 or IPv6:
struct sockaddr* sa = 
                (struct sockaddr *)&remoteAddr;

void* sinAddress = nullptr;

if( sa->sa_family != AF_INET )
  {
  sinAddress = 
          &(((struct sockaddr_in*)sa)->sin_addr);
  }
else
  {
  // AF_INET6
  sinAddress = 
      &(((struct sockaddr_in6*)sa)->sin6_addr );
  }

const Int32 bufLast = 1024; 
char returnS[bufLast];

// Read this and change it to the right struct.
// Not the struct pointer.

// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_ntop

// In WS2tcpip.h
inet_ntop( sa->sa_family,
            &sa,
            returnS, sizeof( returnS ) );

#pragma clang diagnostic pop

for( Int32 count = 0; count < bufLast; count++ )
  {
  if( returnS[count] == 0 )
    break;

  fromCBuf.appendChar( returnS[count] );
  }


//   printf("server: got connection from %s\n", s);

errorBuf.appendChars(
                "Accepted a socket.\n" );

return acceptSock;
}



/*
====
// IPv4 or IPv6:
void* SocketsApi::getInAddress(
                           struct sockaddr *sa )
{
if( sa->sa_family == AF_INET )
  {
  return &(((struct sockaddr_in*)sa)->sin_addr);
  }

return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
*/



Int32 SocketsApi::sendBuf(
                   const SocketCpp sendToSock,
                   const CharBuf& sendBuf,
                   CharBuf& errorBuf )
{
if( sendToSock == INVALID_SOCKET )
  {
  errorBuf.appendChars(
   "SocketsWin sendBuf() sendToSock is invalid.\n" );

  return -1;
  }

const Int32 howMany = sendBuf.getLast();
Int32 result = send( sendToSock,
                     sendBuf.getBufPoint(),
                     howMany,
                     0 );

if( result == SOCKET_ERROR )
  {
  errorBuf.appendChars(
             "SocketsApi sendBuf() error.\n" );
  // WSAGetLastError());
  closesocket( sendToSock );
  return -1;
  }

// How many did it actually send?
return result;
}



bool SocketsApi::receiveBuf(
                   const Uint64 recSock,
                   CharBuf& recCharBuf,
                   CharBuf& errorBuf )
{
if( recSock == INVALID_SOCKET )
  {
  errorBuf.appendChars(
   "SocketsApi receiveBuf() recSock is invalid.\n" );

  return false;
  }

const Int32 bufLen = 1024 * 32;
// On the stack.

char recBuf[bufLen];

// Keep reading for a reasonable length of time.
for( Int32 loops = 0; loops < 2; loops++ )
  {
  Int32 result = recv( recSock, recBuf,
                       bufLen, 0 );

  if( result == 0 )
    {
    // The connection was _gracefully_ closed.
    errorBuf.appendChars(
            "receiveBuf() connection closed.\n" );

    return false;
    }

  if( result < 0 )
    {
    // EAGAIN or EWOULDBLOCK

    Int32 error = WSAGetLastError();
    errorBuf.appendChars( "socket recv error is: " );
    Str errorS( error );
    errorBuf.appendStr( errorS );
    errorBuf.appendChars( "\n" );

    // This might just be saying it would block.
    return false;
    }

  for( Int32 count = 0; count < result; count++ )
    recCharBuf.appendChar( recBuf[count] );

  }

return true;
}



/*
Error codes:

WSA_INVALID_HANDLE
6
Specified event object handle is invalid.

WSA_NOT_ENOUGH_MEMORY
8
Insufficient memory available.


WSA_INVALID_PARAMETER
87
An application used a Windows Sockets function
 which directly maps to a Windows function.
 The Windows function is indicating a problem
 with one or more parameters.


WSA_OPERATION_ABORTED
995
Overlapped operation aborted.

WSA_IO_INCOMPLETE
996
Overlapped I/O event object not in signaled state.

WSA_IO_PENDING
997
Overlapped operations will complete later.

WSAEINTR
10004
Interrupted function call.
A blocking operation was interrupted by a call
 to WSACancelBlockingCall.

WSAEBADF
10009
File handle is not valid.

WSAEACCES
10013
Permission denied.

WSAEFAULT
10014
Bad address.

WSAEINVAL
10022
Invalid argument.


WSAEMFILE
10024
Too many open files.

WSAEWOULDBLOCK
10035
It is normal for WSAEWOULDBLOCK to be reported
 as the result from calling connect on a
 nonblocking SOCK_STREAM socket, since some
 time must elapse for the connection to be
 established.

WSAEINPROGRESS
10036
Operation now in progress.

WSAEALREADY
10037
Operation already in progress.

WSAENOTSOCK
10038
Socket operation on nonsocket.

WSAEDESTADDRREQ
10039
Destination address required.

WSAEMSGSIZE
10040
Message too long.

WSAEPROTOTYPE
10041
Protocol wrong type for socket.

WSAENOPROTOOPT
10042
Bad protocol option.

WSAEPROTONOSUPPORT
10043
Protocol not supported.

WSAESOCKTNOSUPPORT
10044
Socket type not supported.

WSAEOPNOTSUPP
10045
Operation not supported.

WSAEPFNOSUPPORT
10046
Protocol family not supported.

WSAEAFNOSUPPORT
10047
Address family not supported by protocol family.

WSAEADDRINUSE
10048
Address already in use.

WSAEADDRNOTAVAIL
10049
Cannot assign requested address.

WSAENETDOWN
10050
Network is down.

WSAENETUNREACH
10051
Network is unreachable.

WSAENETRESET
10052
Network dropped connection on reset.

WSAECONNABORTED
10053
Software caused connection abort.

WSAECONNRESET
10054
Connection reset by peer.

WSAENOBUFS
10055
No buffer space available.

WSAEISCONN
10056
Socket is already connected.

WSAENOTCONN
10057
Socket is not connected.

WSAESHUTDOWN
10058
Cannot send after socket shutdown.

WSAETOOMANYREFS
10059
Too many references.
Too many references to some kernel object.

WSAETIMEDOUT
10060
Connection timed out.

WSAECONNREFUSED
10061
Connection refused.

WSAELOOP
10062
Cannot translate name.
Cannot translate a name.

WSAENAMETOOLONG
10063
Name too long.
A name component or a name was too long.

WSAEHOSTDOWN
10064
Host is down.

WSAEHOSTUNREACH
10065
No route to host.

WSAENOTEMPTY
10066
Directory not empty.
Cannot remove a directory that is not empty.

WSAEPROCLIM
10067
Too many processes.

WSAEUSERS
10068
User quota exceeded.
Ran out of user quota.

WSAEDQUOT
10069
Disk quota exceeded.
Ran out of disk quota.

WSAESTALE
10070
Stale file handle reference.
The file handle reference is no longer available.

WSAEREMOTE
10071
Item is remote.
The item is not available locally.

WSASYSNOTREADY
10091
Network subsystem is unavailable.

WSAVERNOTSUPPORTED
10092
Winsock.dll version out of range.

WSANOTINITIALISED
10093
Successful WSAStartup not yet performed.

WSAEDISCON
10101
Graceful shutdown in progress.

WSAENOMORE
10102
No more results.

WSAECANCELLED
10103
Call has been canceled.

WSAEINVALIDPROCTABLE
10104
Procedure call table is invalid.

WSAEINVALIDPROVIDER
10105
Service provider is invalid.

WSAEPROVIDERFAILEDINIT
10106
Service provider failed to initialize.

WSASYSCALLFAILURE
10107
System call failure.
A system call that should never fail has failed.

WSASERVICE_NOT_FOUND
10108
Service not found.
No such service is known.

WSATYPE_NOT_FOUND
10109
Class type not found.
The specified class was not found.

WSA_E_NO_MORE
10110
No more results.

WSA_E_CANCELLED
10111
Call was canceled.

WSAEREFUSED
10112
Database query was refused.

WSAHOST_NOT_FOUND
11001
Host not found.
No such host is known.

WSATRY_AGAIN
11002
Nonauthoritative host not found.

WSANO_RECOVERY
11003
This is a nonrecoverable error.

WSANO_DATA
11004
Valid name, no data record of requested type.

WSA_QOS_RECEIVERS
11005
QoS receivers.
At least one QoS reserve has arrived.

WSA_QOS_SENDERS
11006
QoS senders.
At least one QoS send path has arrived.

WSA_QOS_NO_SENDERS
11007
No QoS senders.
There are no QoS senders.

WSA_QOS_NO_RECEIVERS
11008
QoS no receivers.
There are no QoS receivers.

WSA_QOS_REQUEST_CONFIRMED
11009
QoS request confirmed.
The QoS reserve request has been confirmed.

WSA_QOS_ADMISSION_FAILURE
11010
QoS admission error.
A QoS error occurred due to lack of resources.

WSA_QOS_POLICY_FAILURE
11011
QoS policy failure.

WSA_QOS_BAD_STYLE
11012
QoS bad style.
An unknown or conflicting QoS style was encountered.

WSA_QOS_BAD_OBJECT
11013
QoS bad object.

WSA_QOS_TRAFFIC_CTRL_ERROR
11014
QoS traffic control error.

WSA_QOS_GENERIC_ERROR
11015
QoS generic error.
A general QoS error.

WSA_QOS_ESERVICETYPE
11016
QoS service type error.

WSA_QOS_EFLOWSPEC
11017
QoS flowspec error.

WSA_QOS_EPROVSPECBUF
11018
Invalid QoS provider buffer.
An invalid QoS provider-specific buffer.

WSA_QOS_EFILTERSTYLE
11019
Invalid QoS filter style.
An invalid QoS filter style was used.

WSA_QOS_EFILTERTYPE
11020
Invalid QoS filter type.
An invalid QoS filter type was used.

WSA_QOS_EFILTERCOUNT
11021
Incorrect QoS filter count.

WSA_QOS_EOBJLENGTH
11022
Invalid QoS object length.

WSA_QOS_EFLOWCOUNT
11023
Incorrect QoS flow count.

WSA_QOS_EUNKOWNPSOBJ
11024
Unrecognized QoS object.

WSA_QOS_EPOLICYOBJ
11025
Invalid QoS policy object.

WSA_QOS_EFLOWDESC
11026
Invalid QoS flow descriptor.

WSA_QOS_EPSFLOWSPEC
11027
Invalid QoS provider-specific flowspec.

WSA_QOS_EPSFILTERSPEC
11028
Invalid QoS provider-specific filterspec.

WSA_QOS_ESDMODEOBJ
11029
Invalid QoS shape discard mode object.

WSA_QOS_ESHAPERATEOBJ
11030
Invalid QoS shaping rate object.

WSA_QOS_RESERVED_PETYPE
11031
Reserved policy QoS element type.


*/
