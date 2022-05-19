// Copyright Eric Chauvin 2022



// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#include "Server.h"
#include "SocketsApiWin.h"
// #include "SocketsApiLinux.h"
#include "../CppBase/StIO.h"
#include "../CppBase/Threads.h"
#include "../WinApi/Signals.h"
// #include "../LinuxApi/Signals.h"



Server::Server( void )
{
mainSocket = SocketsApi::InvalidSocket;
}



Server::Server( const Server& in )
{
mainSocket = SocketsApi::InvalidSocket;

// Make the compiler think the in value is
// being used.
if( in.testForCopy == 123 )
  return;

const char* showS = "The Server copy"
         " constructor should not get called.";

throw showS;
}



Server::~Server( void )
{
}



bool Server::startServer( const char* port )
{
StIO::putS( "Starting server." );

mainSocket = SocketsApi::openServer(
                            port, showBuf );

if( mainSocket == SocketsApi::InvalidSocket )
  {
  StIO::putCharBuf( showBuf );
  showBuf.clear();
  return false;
  }

StIO::putCharBuf( showBuf );
showBuf.clear();

StIO::putS( "Server is listening." );

mainLoop();

return true;
}


void Server::mainLoop( void )
{
while( true )
  {
  if( Signals::getControlCSignal())
    {
    StIO::putS( "Closing server on Ctrl-C." );
    // close all current open sockets.
    break;
    }

  SocketCpp acceptedSock =
                 SocketsApi::acceptConnect(
                                  mainSocket,
                                  showBuf );

  if( acceptedSock == SocketsApi::InvalidSocket )
    {
    // There is nothing new trying to connect now.
    // Do this dynamically and adjust it to sleep
    // more or less or none if it's busy.
    Int32 milliSec = 1000;
    Threads::sleep( milliSec );
    continue;
    }

  }

// Close the server.
SocketsApi::closeSocket( mainSocket );
StIO::putS( "Closed server." );
}



void Server::addNewConnections( void )
{
// Add up to max new clients in one loop.
const Int32 max = 20;
for( Int32 count = 0; count < max; count++ )
  {
  SocketCpp acceptedSock =
                 SocketsApi::acceptConnect(
                                  mainSocket,
                                  showBuf );

  if( acceptedSock == SocketsApi::InvalidSocket )
    return;

  }
}
