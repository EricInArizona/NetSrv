// Copyright Eric Chauvin 2022


// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html



// A good tutorial:
// https://beej.us/guide/bgnet/html/



////////////////////
// Some notes:

// IPv4 addresses are 4 bytes.  They are shown
// as a string delimited with dots like:
// 123.45.67.89

// The loopback address is: 127.0.0.1
// #define INADDR_LOOPBACK  0x7f000001
//                          0x7f 00 00 01
// So that's 32 bits.

// IPv6 addresses are shown as two bytes
// separated by colons like ab12:45cd:39fd: and
// so on up to 16 bytes.  128 bits.
// Two colons together means the bytes are zero
// between the two colons.
// ab12::45cd:39fd...

// ::1 is the loopback address.
// Meaning all leading zeros and then a 1.


// Get the right byte order:
// htons() h to n s host to network short
// htonl() host to network long
// ntohs() network to host short
// ntohl() network to host long


///////////////////////////
// Now for the code:


#include "SocketsApiWin.h"
// #include "SocketsApiLinux.h"
#include "../CppBase/Casting.h"
#include "../CppBase/StIO.h"

// I hate to have to put a #define statement
// into any of my code.  But if you need to
// include the Windows.h file then
// you have to define

#define WIN32_LEAN_AND_MEAN

// So that it doesn't include winsock.h.
// And of course this would only be done in
// a .cpp file and never in a header file

// This might or might not be included with
// Winsock2.h already.
// #include <windows.h>


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


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wcast-align"


// I'm making a class that is hidden inside a
// .cpp file so that it can only be seen inside
// this compilation unit.  It does not get
// #included in anything else.  So nothing
// else sees it.  It's not an inner class,
// a nested class, but a privately used class.


class GetAddress
  {
  private:
  struct addrinfo* resultList = nullptr;
  struct addrinfo* nextResultPt = nullptr;
  struct addrinfo hints;

  public:
  GetAddress( void );
  ~GetAddress( void );
  bool getAddressInfo( const char* domain,
                       const char* port,
                       const bool isServer,
                       const bool useIPv4Only );

  inline sockaddr* getSockAddrPt( void )
    {
    if( nextResultPt == nullptr )
      return nullptr;

    return nextResultPt->ai_addr;
    }

  inline Uint64 getAddrLength( void )
    {
    if( nextResultPt == nullptr )
      return 0;

    return nextResultPt->ai_addrlen;
    }

  inline const char* getCanonName( void )
    {
    if( nextResultPt == nullptr )
      return nullptr;

    return nextResultPt->ai_canonname;
    }

  bool moveToNextAddr( void );

  static Str getAddressStr( struct sockaddr* sa );

  inline Int32 getFamily( void )
    {
    if( nextResultPt == nullptr )
      return -1;

    return nextResultPt->ai_family;
    }

  inline Int32 getSockType( void )
    {
    if( nextResultPt == nullptr )
      return -1;

    return nextResultPt->ai_socktype;
    }

  inline Int32 getProtocol( void )
    {
    if( nextResultPt == nullptr )
      return -1;

    return nextResultPt->ai_protocol;
    }



  };


GetAddress::GetAddress( void )
{
}


GetAddress::~GetAddress( void )
{
if( resultList != nullptr )
  {
  freeaddrinfo( resultList );
  resultList = nullptr;
  }
}


bool GetAddress::getAddressInfo(
                           const char* domain,
                           const char* port,
                           const bool isServer,
                           const bool useIPv4Only )
{
// StIO::putS( "getAddressInfo called." );

// result points to the first struct in the
// linked list.
// The getaddrinfo() function creates the space
// to put the data it gets, and then result
// points to that space it created. So calling
// freeaddrinfo( result ) frees up that space
// that it created.

// memset( &hints, 0, sizeof( hints ));
ZeroMemory( &hints, sizeof( hints ) );

// It's unspecified so it's either IPV4 or IPV6.
// or AF_UNSPEC.
// AF_INET6 is IPv6.
if( useIPv4Only )
  hints.ai_family = AF_INET; // IPv4
else
  hints.ai_family = AF_UNSPEC;

hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;


// if( domain == nullptr )

if( isServer )
  hints.ai_flags = AI_PASSIVE;


// Port 443 for https.

Int32 status = getaddrinfo(
              domain, // "www.thedomain.com"
              port, // "443" "https", "ftp", etc.
              // Pass it the address of the
              // hints struct.
              &hints,
              // Pass it the address of the
              // pointer so that it can tell
              // you where it allocated memory.
              &resultList );

if( status != 0 )
  {
  StIO::putS( "SocketsApi getaddrinfo error." );

  // 11001 is host not found.
  Int32 error = WSAGetLastError();

  if( error == WSAHOST_NOT_FOUND )
    StIO::putS( "Host not found." );

  return false;
  }

if( resultList == nullptr )
  StIO::putS( "resultList can't be right." );

// Set it to the first result.
nextResultPt = resultList;
return true;
}



bool GetAddress::moveToNextAddr( void )
{
if( nextResultPt == nullptr )
  return false;

nextResultPt = nextResultPt->ai_next;
if( nextResultPt == nullptr )
  return false;

return true;
}



Str GetAddress::getAddressStr(
                             struct sockaddr* sa )
{
CharBuf fromCBuf;

// struct in_addr {
// Uint32 s_addr;
// };

// Size of struct sockaddr_storage.
// #define _SS_SIZE 128

//struct sockaddr_storage {
// sa_family_t ss_family;
// filler up to the 128 bytes.
// };

// For IPv4.
// struct sockaddr_in {
//        short   sin_family;
//        u_short sin_port;
//        struct  in_addr sin_addr;
//        char    sin_zero[8];
// };

// For IPv6.
// struct sockaddr_in6 {
//        u_short   sin6_family;
//        u_short sin6_port;
//        u_short sin6_flowinfo;
//        struct  in6_addr sin6_addr;
//        Uint32 sin6_scope_id;
// };

// getpeername()

void* sinAddress = nullptr;

if( sa == nullptr )
  {
  StIO::putS( "The sa was null in getAddressStr." );
  return Str( "" );
  }

if( !( (sa->sa_family == AF_INET) ||
       (sa->sa_family == AF_INET6)) )
  {
  StIO::putS( "The sa_family is not right." );
  return Str( "" );
  }

if( sa->sa_family == AF_INET )
  {
  // StIO::putS( "IPv4 address:" );
  sinAddress =
         &(((struct sockaddr_in*)sa)->sin_addr);
  }
else
  {
  // AF_INET6
  // StIO::putS( "IPv6 address:" );
  sinAddress =
    &(((struct sockaddr_in6*)sa)->sin6_addr );
  }

const Int32 bufLast = 1024;
char returnS[bufLast];

// inet_pton()

// In WS2tcpip.h
if( nullptr == inet_ntop( sa->sa_family,
            sinAddress,
            returnS, sizeof( returnS ) ))
  {
  StIO::putS( "Error getting the address string." );
  return Str( "" );
  }

for( Int32 count = 0; count < bufLast; count++ )
  {
  if( returnS[count] == 0 )
    break;

  fromCBuf.appendChar( returnS[count] );
  }

StIO::putCharBuf( fromCBuf );
StIO::putS( " " );

return fromCBuf.getStr();
}




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
// In Winsock2.h:
// #define INVALID_SOCKET  (SOCKET)(~0)
// So this should be all ones.

if( ~(unsigned long long)0 == INVALID_SOCKET )
  return 0;

// This is unreachable code because the above
// line is true.
return 0;
}
*/




void SocketsApi::closeSocket( SocketCpp toClose )
{
if( toClose == InvalSock )
  {
  StIO::putS( "Closing an invalid socket." );
  return;
  }
// For Windows.
// returns zero on no error.

closesocket( toClose );
// Linux uses close();
}



void SocketsApi::shutdownRead( SocketCpp toClose )
{
if( toClose == InvalSock )
  return;

// What is the receive symbol?
shutdown( toClose, SD_RECEIVE );
// SD_SEND   SD_BOTH
}



SocketCpp SocketsApi::connectClient(
                        const char* domain,
                        const char* port )
{
GetAddress getAddress;

// Port 443 for https.

if( !getAddress.getAddressInfo( domain, port,
                                false, false ))
  {
  StIO::putS( "connect() could not get address." );
  return InvalSock;
  }

// SOCKET clientSocket = INVALID_SOCKET;
SocketCpp clientSocket = InvalSock;

// Try the possible connections.
// I don't want to let it go wild on corrupted
// data, so it will try up to five of them.
// But it should usually be the first one anyway.

CharBuf fromCBuf;

for( Int32 count = 0; count < 5; count++ )
  {
  StIO::putS( "Client connect address:" );
  fromCBuf.clear();

  sockaddr* addr = getAddress.getSockAddrPt();

  Str showAddr = GetAddress::getAddressStr( addr );
  if( showAddr.getSize() == 0 )
    {
    if( !getAddress.moveToNextAddr())
      return InvalSock;

    continue;
    }

  fromCBuf.appendStr( showAddr );
  StIO::putCharBuf( fromCBuf );
  StIO::putS( " " );

  clientSocket = socket( getAddress.getFamily(),
                     getAddress.getSockType(),
                     getAddress.getProtocol() );

  if( clientSocket == INVALID_SOCKET )
    {
    StIO::putS( "No sockets left for connect." );
    return InvalSock;
    }

  // If it's non blocking then will it block
  // on connect?
  // Do you have to do this before you connect()?
  if( !setNonBlocking( clientSocket ))
    {
    StIO::putS( "False on setNonBlocking." );
    closesocket( clientSocket );
    clientSocket = InvalSock;
    if( !getAddress.moveToNextAddr())
      {
      StIO::putS( "No more addresses to try." );
      return InvalSock;
      }

    continue;
    }

  Int32 connectResult = connect( clientSocket,
             addr,
             Casting::U64ToI32(
             getAddress.getAddrLength() ));

  if( connectResult == SOCKET_ERROR )
    {
    StIO::putS( "Could not connect socket." );
    closesocket( clientSocket );
    clientSocket = INVALID_SOCKET;
    if( !getAddress.moveToNextAddr())
      {
      StIO::putS( "No more addresses to try." );
      return InvalSock;
      }

    continue;
    }

  // If it got this far it got a connected
  // socket.
  break;
  }

if( clientSocket == InvalSock )
  {
  StIO::putS( "Connected to an invalid socket?" );
  return InvalSock;
  }

StIO::putS( "Connected to:" );
StIO::putS( domain );

return clientSocket;
}




bool SocketsApi::setNonBlocking(
                            const SocketCpp toSet )
{
// 0 is blocking.
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

// Here are the macros it uses.

// This high bit of a signed number is dealt
// with in Casting::u32ToI32ForMacro().
// #define IOC_IN          0x80000000

// #define IOCPARM_MASK    0x7f
// #define _IOW(x,y,t)
//   (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)
//                 <<16)|((x)<<8)|(y))
// #define FIONBIO     _IOW('f', 126, u_long)
// #define _IOW(x,y,t)
//               (IOC_IN|(((long)sizeof(t)
//               &IOCPARM_MASK)<<16)|((x)<<8)|(y))


Int32 status = ioctlsocket( toSet,
                    Casting::u32ToI32ForMacro(
                    FIONBIO ),
                    // FIONBIO,
                    &iMode );
if( status != 0 )
  {
  Int32 error = WSAGetLastError();
  StIO::putS( "socket ioctl failed." );
  StIO::printF( "Error is: " );
  StIO::printFD( error );
  StIO::printF( "\n" );
  return false;
  }

return true;
}


SocketCpp SocketsApi::openServer(
                          const char* ipAddress,
                          const char* port,
                          bool useIPv4Only )
{
// If you specify "localhost" then specify
// that it's IPv4 because otherwise you
// might get the localhost address ::1 for
// IPv6.  Then if you try to connect a client
// to 127.0.0.1 it won't find it.

// Str addrS( ipAddress );
// if( addrS.getSize() == 0 )
  // If it's equal a string...


GetAddress getAddress;

if( !getAddress.getAddressInfo( ipAddress,
                                port,
                                true,
                                useIPv4Only ))
  {
  StIO::putS(
       "openServer() could not get address." );
  return InvalSock;
  }

CharBuf fromCBuf;
Str showAddr = GetAddress::getAddressStr( 
                       getAddress.getSockAddrPt() );
if( showAddr.getSize() == 0 )
  {
  StIO::putS( "No IP address for the server." );
  // if( !getAddress.moveToNextAddr())

  return InvalSock;
  // continue;
  }

StIO::putS( "Server IP address:" );
fromCBuf.appendStr( showAddr );
StIO::putCharBuf( fromCBuf );
StIO::putS( " " );

// The stats and hacking info for disallowed
// IP addresses and things.

// The domain can be "localhost".
// An empty string means all addresses on the local
// computer are returned.
// Make it the IP address if you wanted to bind
// it to a specific IP address.

// SOCKET serverSocket = INVALID_SOCKET;
SocketCpp serverSocket = socket(
                      getAddress.getFamily(),
                      getAddress.getSockType(),
                      getAddress.getProtocol() );

if( serverSocket == InvalSock )
  {
  StIO::putS( "Server no sockets." );
  // WSAGetLastError());
  return InvalSock;
  }

if( 0 != bind( serverSocket, getAddress.
               getSockAddrPt(),
               Casting::U64ToI32(
               getAddress.getAddrLength() )))
  {
  closeSocket( serverSocket );
  StIO::putS( "Server bind error." );
  return InvalSock;
  }

// The backlog could be several hundred in Windows.
// What is it in Linux?
if( 0 != listen( serverSocket, 100 ))
  {
  closeSocket( serverSocket );
  StIO::putS( "Server listen error." );
  return InvalSock;
  }

if( !setNonBlocking( serverSocket ))
  {
  StIO::putS( "setNonBlocking returned false." );
  return InvalSock;
  }

StIO::putS( "Got a server socket." );

return serverSocket;
}



/*
// Also see poll().
bool SocketsWin::checkSelect(
                         SocketCpp servSock )
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
StIO::putS(
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
                         CharBuf& fromCBuf )
{
fromCBuf.clear();

// sockaddr_storage is big enough to hold IPv6
// or IPv4.
struct sockaddr_storage remoteAddr;
Int32 addrSize = sizeof( remoteAddr );

// if( !checkSelect( servSock ))
  // return InvalSock;

SocketCpp acceptSock = accept( servSock,
              (struct sockaddr *)&remoteAddr,
              &addrSize );

if( acceptSock == InvalSock )
  {
  Int32 error = WSAGetLastError();
  // if( error == EAGAIN )

  if( error == WSAEWOULDBLOCK )
    {
    // StIO::putS( "Socket would block." );
    return InvalSock;
    }

  StIO::putS( "socket accept error is: " );
  StIO::printFD( error );
  StIO::printF( "\n" );
  return InvalSock;
  }

StIO::putS( "Accepted a socket at top." );

// IPv4 or IPv6:
struct sockaddr* sa =
                (struct sockaddr *)&remoteAddr;

// StIO::putS( "Accepted a socket from." );
// GetAddress::showAddress( sa, fromCBuf );

Str showAddr = GetAddress::getAddressStr( 
                       getAddress.getSockAddrPt() );
if( showAddr.getSize() == 0 )
  {
  StIO::putS( "No IP address for the server." );
  // if( !getAddress.moveToNextAddr())

  return InvalSock;
  // continue;
  }

=====
StIO::putS( "Server IP address:" );
fromCBuf.appendStr( showAddr );
StIO::putCharBuf( fromCBuf );
StIO::putS( " " );

return acceptSock;
}



Int32 SocketsApi::sendBuf(
                   const SocketCpp sendToSock,
                   const CharBuf& sendBuf )
{
if( sendToSock == INVALID_SOCKET )
  {
  StIO::putS( "sendBuf() sendToSock is invalid." );
  return -1;
  }

const Int32 howMany = sendBuf.getLast();
Int32 result = send( sendToSock,
                     sendBuf.getBufPoint(),
                     howMany,
                     0 );

if( result == SOCKET_ERROR )
  {
  StIO::putS( "SocketsApi sendBuf() error." );
  Int32 error = WSAGetLastError();
  StIO::printF( "Error is: " );
  StIO::printFD( error );
  StIO::printF( "\n" );
  closesocket( sendToSock );
  return -1;
  }

// How many did it actually send?
return result;
}



bool SocketsApi::receiveBuf(
                   const Uint64 recSock,
                   CharBuf& recCharBuf )
{
if( recSock == InvalSock )
  {
  StIO::putS( "receiveBuf() recSock is invalid." );
  return false;
  }

const Int32 bufLen = 1024 * 32;
// On the stack.
char recBuf[bufLen];

Int32 result = recv( recSock, recBuf,
                              bufLen, 0 );

if( result == 0 )
  {
  // The connection was _gracefully_ closed.
  StIO::putS( "receiveBuf() connection closed." );
  return false;
  }

if( result < 0 )
  {
  // EAGAIN or EWOULDBLOCK

  Int32 error = WSAGetLastError();
  StIO::putS( "socket recv error is: " );
  StIO::printFD( error );
  StIO::printF( "\n" );

  // This might just be saying it would block.
  return false;
  }

for( Int32 count = 0; count < result; count++ )
  recCharBuf.appendChar( recBuf[count] );

return true;
}



#pragma clang diagnostic pop
