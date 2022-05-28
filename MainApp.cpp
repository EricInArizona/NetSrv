// Copyright Eric Chauvin, 2021 - 2022.


// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#include "../CppBase/StIO.h"
#include "../CppBase/Str.h"
// #include "../LinuxApi/SetStack.h"
#include "../CppBase/Casting.h"
#include "../CppBase/Threads.h"

#include "../WinApi/Signals.h"
// #include "../LinuxApi/Signals.h"



#include "MainApp.h"



// int MainApp::mainLoop( int argc, char* argv[] )
int MainApp::mainLoop( void )
{
Int32 delay = 200; // milliseconds.

// If it's out of RAM and can't allocate the
// memory for a new object then it throws an
// exception.
// set_new_handler()

try
{
// Throws an exception if things are not right.
BasicTypes::thingsAreRight();


StIO::putS( "Programming by Eric Chauvin." );
StIO::printF( "Version date: " );
StIO::putS( getVersionStr() );
StIO::printF( "\n" );

// For Linux:
// Int32 stackSize = SetStack::getSize();
// Str showS( stackSize );
// mainIO.appendChars( "Stack size: " );
// mainIO.appendStr( showS );
// mainIO.appendChars( "\n\n" );

Signals::setupControlCSignal();
Signals::setupFpeSignal();
Signals::setupIllegalOpSignal();
Signals::setupBadMemSignal();


runServer();


StIO::putS( "End of main app." );

Threads::sleep( delay );

return 0;
}
catch( const char* in )
  {
  StIO::putS( "Exception in main loop.\n" );
  StIO::putS( in );
  Threads::sleep( delay );
  return 1;
  }

catch( ... )
  {
  const char* in = "An unknown exception"
                   " happened in the main loop.\n";

  StIO::putS( in );
  Threads::sleep( delay );
  return 1;
  }
}


void MainApp::runServer( void )
{
/*
Uint64 testSocket = SocketsApi::connectClient(
                     "www.durangoherald.com",
                     "443" );
if( testSocket == 0 )

*/


if( !server.startServer( "443" ))
  return;

server.mainLoop();
}
