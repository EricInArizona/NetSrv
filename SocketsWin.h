// Copyright Eric Chauvin 2022



// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#pragma once


// A good tutorial:
// https://beej.us/guide/bgnet/html/

// DNS over HTTPS
// https://en.wikipedia.org/wiki/DNS_over_HTTPS


// The SocketCpp type is in BasicTypes.h

#include "../CppBase/BasicTypes.h"
#include "../CppBase/CharBuf.h"
// #include "../CppBase/Str.h"
// #include "../CppBase/RangeC.h"


class SocketsWin
  {
  private:
  Int32 testForCopy = 123;

  public:
  SocketsWin( void );
  SocketsWin( const SocketsWin &in );
  ~SocketsWin( void );
  static void closeSocket( SocketCpp toClose );

  static SocketCpp openClient( const char* domain,
                               const char* port,
                               CharBuf& errorBuf );

  static SocketCpp openServer( const char* port,
                               CharBuf& errorBuf );

  static bool checkSelect( SocketCpp servSock,
                           CharBuf& errorBuf );

  static SocketCpp acceptConnect(
                         SocketCpp servSock,
                         CharBuf& errorBuf );

  static Int32 sendBuf( const SocketCpp sendToSock,
                        const CharBuf& sendBuf,
                        CharBuf& errorBuf );

  static bool receiveBuf( const SocketCpp recSock,
                          CharBuf& recCharBuf,
                          CharBuf& errorBuf );

  };
