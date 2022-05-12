// Copyright Eric Chauvin 2022



// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#include "SocketClient.h"


/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
*/

#include "../CppBase/Casting.h"




SocketClient::SocketClient( void )
{
arraySize = 1024 * 2;
cArray = new char[Casting::i32ToU64( arraySize )];
}



SocketClient::SocketClient( const SocketClient &in )
{
// Make the compiler think the in value is
// being used.
if( in.testForCopy == 123 )
  return;

// Don't copy a giant buffer.
const char* showS = "The SocketClient copy constructor"
         " should not be getting called.\n";

throw showS;
}



SocketClient::~SocketClient( void )
{
delete[] cArray;
}



/*
void SocketClient::doSomething( void )
{
  // Int32 sockfd = 0; // File descriptor.
  // Int32 newsockfd = 0;
  // Int32 portno = 0;
  // Int32 clilen = 0;
  // Int32 n = 0;

//////
struct sockaddr_in
{
  short   sin_family; // must be AF_INET
  u_short sin_port;
  struct  in_addr sin_addr;
  char    sin_zero[8]; // Not used, must be zero
};


// struct sockaddr_in serv_addr;
// struct sockaddr_in cli_addr;


}
*/
