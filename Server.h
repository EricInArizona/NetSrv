// Copyright Eric Chauvin 2022



// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#pragma once


#include "../CppBase/BasicTypes.h"
#include "../CppBase/CharBuf.h"
#include "SrvClient.h"



class Server
  {
  private:
  Int32 testForCopy = 123;
  // There is one main server socket.
  SocketCpp mainSocket = 0;
  CharBuf showBuf;

  public:
  Server( void );
  Server( const Server &in );
  ~Server( void );
  bool startServer( const char* port );
  void mainLoop( void );

  };
