// Copyright Eric Chauvin 2022



// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#pragma once


#include "../CppBase/BasicTypes.h"


// This is the server's view of one client
// socket.

class SrvClient
  {
  private:
  Int32 testForCopy = 123;
  SocketCpp mainSocket = 0;

  public:
  SrvClient( void );
  SrvClient( const SrvClient &in );
  ~SrvClient( void );
  void setSocket( const SocketCpp toSet );

  };
