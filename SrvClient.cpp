// Copyright Eric Chauvin 2022



// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#include "SrvClient.h"



SrvClient::SrvClient( void )
{
mainSocket = 0;
}



SrvClient::SrvClient( const SrvClient& in )
{
mainSocket = 0;

// Make the compiler think the in value is
// being used.
if( in.testForCopy == 123 )
  return;

const char* showS = "The SrvClient copy"
         " constructor should not get called.";

throw showS;
}



SrvClient::~SrvClient( void )
{
}


void SrvClient::setSocket( const SocketCpp toSet )
{
mainSocket = toSet;
}
