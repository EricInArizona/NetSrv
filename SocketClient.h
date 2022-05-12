// Copyright Eric Chauvin 2022



// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html



#pragma once


// This file has a lot of notes on the bottom from
// my old C# stuff.
// Things on my ToDo list.




// Port 443 for https.
// The client makes a request first.


#include "../CppBase/BasicTypes.h"
// #include "../CppBase/FileIO.h"



class SocketClient
  {
  private:
  Int32 testForCopy = 123;
  // FileIO mainIO;
  Int32 arraySize = 0;
  char* cArray;  // Uint8?



  public:
  SocketClient( void );
  SocketClient( const SocketClient &in );
  ~SocketClient( void );
  // void doSomething( void );



  };


/*

// Programming by Eric Chauvin.
// Notes on this source code are at:
// http://eric7apps.blogspot.com/

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Net.Sockets;
using System.Security.Cryptography;



// RFC 5246:
// The Transport Layer Security (TLS) Protocol Version 1.2
// https://tools.ietf.org/html/rfc5246

// https://en.wikipedia.org/wiki/Transport_Layer_Security



namespace ExampleServer
{
  class CustomerTLSClient
  {
  private NetworkStream NetStream;
  private TcpClient Client;
  private string StatusString = "";
  private ECTime LastReadWriteTime;
  private byte[] RawBuffer;
  private int RawBufferStart = 0;
  private int RawBufferEnd = 0;
  private byte[] TLSOuterRecordBuffer;
  private int TLSOuterRecordBufferLast = 0;
  private int OuterTLSContentType = 0;
  private int OuterTLSVersionMajor = 0;
  private int OuterTLSVersionMinor = 0;
  private int OuterTLSLength = 0;
  // Apparently some implementations go beyond this record length limit.
  // The length is a 16 bit number and so it could go up to 64K.
  private const int MaximumTLSRecordLength = 16384 + 5; // RFC says length must not exceed 2^14.
  private const int RawBufferLength = 1024 * 128;
  private RNGCryptoServiceProvider CryptoRand;
  private ProcessHandshake Handshake;
  private string DomainName = "";



  private CustomerTLSClient()
    {
    }


  internal CustomerTLSClient( string UseDomainName )
    {
    DomainName = UseDomainName;
    RawBuffer = new byte[RawBufferLength];
    TLSOuterRecordBuffer = new byte[MaximumTLSRecordLength];

    Handshake = new ProcessHandshake( DomainName );

    Client = new TcpClient();
    Client.ReceiveTimeout = 15 * 1000;
    Client.SendTimeout = 15 * 1000;
    LastReadWriteTime = new ECTime();
    LastReadWriteTime.SetToNow();
    CryptoRand = new RNGCryptoServiceProvider();
    }


  internal DomainX509Record GetX509Record()
    {
    if( Handshake == null )
      return null;

    return Handshake.GetX509Record();
    }


  internal void FreeEverything()
    {
    if( NetStream != null )
      {
      NetStream.Close();
      NetStream = null;
      }

    if( Client != null )
      {
      Client.Close();
      Client = null;
      }
    }



  internal string GetStatusString()
    {
    string Result = StatusString + Handshake.GetStatusString();
    StatusString = "";
    return Result;
    }



  internal double GetLastReadWriteTimeSecondsToNow()
    {
    return LastReadWriteTime.GetSecondsToNow();
    }



  internal bool IsShutDown()
    {
    if( Client == null )
      return true;

    if( !Client.Connected )
      {
      FreeEverything();
      return true;
      }

    return false;
    }



  private int GetAvailable()
    {
    if( IsShutDown())
      return 0;

    try
    {
    return Client.Available;
    }
    catch( Exception )
      {
      FreeEverything();
      return 0;
      }
    }



  internal bool Connect( string ServerIP, int ServerPort )
    {
    try
    {
    Client.Connect( ServerIP, ServerPort );
    }
    catch( Exception Except )
      {
      StatusString += "Could not connect to the server: " + ServerIP + "\r\n";
      StatusString += Except.Message + "\r\n";
      return false;
      }

    // Is this delay needed for a bug?  So Client.GetStream() works?
    // Or is that only for older versions of .NET?
    Thread.Sleep( 100 );

    try
    {
    NetStream = Client.GetStream();
    }
    catch( Exception Except )
      {
      StatusString += "Could not connect to the server (2): " + ServerIP + "\r\n";
      StatusString += Except.Message + "\r\n";
      NetStream = null;
      return false;
      }

    LastReadWriteTime.SetToNow();
    return true;
    }




  private bool WaitForData()
    {
    try
    {
    // Wait while data is not yet here.
    if( DataIsAvailable() )
      return true;

    Thread.Sleep( 100 );

    if( DataIsAvailable() )
      return true;

    return false;

    }
    catch
      {
      return false;
      }
    }




  private bool DataIsAvailable()
    {
    if( NetStream == null )
      return false;

    try
    {
    if( NetStream.DataAvailable )
      return true;

    return false;

    }
    catch
      {
      FreeEverything();
      return false;
      }
    }



  internal bool SendBuffer( byte[] Buffer )
    {
    if( IsShutDown())
      return false;

    if( NetStream == null )
      {
      StatusString += "NetStream is null in SendBuffer().";
      return false;
      }

    try
    {
    NetStream.Write( Buffer, 0, Buffer.Length );
    }
    catch
      {
      StatusString += "Could not send Buffer.";
      return false;
      }

    LastReadWriteTime.SetToNow();
    return true;
    }



  internal bool ProcessOuterMessages()
    {
    // if( IsShutDown())
      // return false;

    ReadToRawBuffer();
    if( ReadNextOuterTLSRecord())
      {
      // There is a record to process.
      if( ProcessOneOuterTLSRecord())
        {

        }
      }

    return true;
    }



  private bool ReadToRawBuffer()
    {
    if( IsShutDown())
      return false;

    try
    {
    if( 0 == GetAvailable())
      return true; // No error.

    if( NetStream == null )
      return false; // NetStream = Client.GetStream();

    if( !DataIsAvailable() )
      return true;

    byte[] TempRawData = new byte[RawBufferLength];
    int BytesRead = NetStream.Read( TempRawData, 0, TempRawData.Length );
    if( BytesRead == 0 )
      return true; // Nothing to read.

    LastReadWriteTime.SetToNow();

    // Notice that _anything_ that a hacker sends is added here.
    for( int Count = 0; Count < BytesRead; Count++ )
      {
      RawBuffer[RawBufferEnd] = TempRawData[Count];
      RawBufferEnd++;
      if( RawBufferEnd >= RawBufferLength )
        RawBufferEnd = 0; // Wrap around in a circle.

      if( RawBufferEnd == RawBufferStart )
        {
        StatusString += "The RawBuffer overflowed in CustomerTLSSocket.";
        // To Do: Send an alert to gracefully end it.
        FreeEverything(); // Close this connection.
        return false;
        }
      }

    return true;

    }
    catch( Exception Except )
      {
      StatusString += "Exception in CustomerTLSClient.ReadToRawBuffer():";
      StatusString += Except.Message;
      FreeEverything();
      return false;
      }
    }



  private bool IsValidContentType( int ToCheck )
    {
    if( (ToCheck == 0x14) || // 20 ChangeCipherSpec
        (ToCheck == 0x15) || // 21 Alert
        (ToCheck == 0x16) || // 22 Handshake
        (ToCheck == 0x17) || // 23 Application
        (ToCheck == 0x18))   // 24 Heartbeat (As in the infamous Heartbleed bug.)
      return true;
    else
      return false;

    }



  private bool ReadNextOuterTLSRecord()
    {
    try
    {
    // Return true if a new record is ready.
    if( RawBufferEnd == RawBufferStart )
      return false; // Nothing to read.

    int HowManyInRawBuffer = RawBufferEnd - RawBufferStart;
    if( HowManyInRawBuffer < 0 ) // If RawBufferEnd wrapped around.
      HowManyInRawBuffer = (RawBufferEnd + RawBufferLength) - RawBufferStart;

    if( HowManyInRawBuffer < 5 )
      {
      // It doesn't even have the length yet.
      return false;
      }

    StatusString += "RawBufferStart: " + RawBufferStart.ToString() + "\r\n";
    StatusString += "RawBufferEnd: " + RawBufferEnd.ToString() + "\r\n";
    StatusString += "HowManyInRawBuffer: " + HowManyInRawBuffer.ToString() + "\r\n";

    int TempStart = RawBufferStart;
    TLSOuterRecordBufferLast = 0;

    // It says that the MAC is done on the whole message including the outer
    // header and a sequence number.

    OuterTLSContentType = RawBuffer[TempStart];
    TLSOuterRecordBuffer[TLSOuterRecordBufferLast] = RawBuffer[TempStart];
    TLSOuterRecordBufferLast++;
    TempStart++;
    if( TempStart >= RawBufferLength )
      TempStart = 0; // Wrap it around.

    if( !IsValidContentType( OuterTLSContentType ))
      {
      StatusString += "Received an invalid content type in CustomerTLSClient.ReadNextOuterTLSRecord().\r\n";
      StatusString += "OuterTLSContentType: " + OuterTLSContentType.ToString() + "\r\n";
      // To Do: Send an alert and gracefully shut it down.
      FreeEverything();
      return false;
      }
    else
      {
      StatusString += "Valid OuterTLSContentType: " + OuterTLSContentType.ToString() + "\r\n";
      }

    // This is only written for TLS version 1.2 (RFC 5246), which means that it's
    // version 3.3 in the version major and minor fields.  But it says in the RFC
    // that "a client that supports multiple versions of TLS may not know what"
    // version will be employed before it receives the ServerHello".  And also
    // it says "TLS servers compliant with this specification MUST accept any
    // value {03,XX} as the record layer version number for ClientHello."

    OuterTLSVersionMajor = RawBuffer[TempStart];
    TLSOuterRecordBuffer[TLSOuterRecordBufferLast] = RawBuffer[TempStart];
    TLSOuterRecordBufferLast++;
    TempStart++;
    if( TempStart >= RawBufferLength )
      TempStart = 0; // Wrap it around.

    if( OuterTLSVersionMajor != 3 )
      {
      StatusString += "Received an invalid major version number in CustomerTLSClient.ReadNextOuterTLSRecord().\r\n";
      // To Do: Send an alert and gracefully shut it down.
      // "protocol_version" alert?
      FreeEverything();
      return false;
      }

    OuterTLSVersionMinor = RawBuffer[TempStart];
    TLSOuterRecordBuffer[TLSOuterRecordBufferLast] = RawBuffer[TempStart];
    TLSOuterRecordBufferLast++;
    TempStart++;
    if( TempStart >= RawBufferLength )
      TempStart = 0; // Wrap it around.

    // Get the high byte of the length.
    OuterTLSLength = RawBuffer[TempStart];
    OuterTLSLength <<= 8;
    TLSOuterRecordBuffer[TLSOuterRecordBufferLast] = RawBuffer[TempStart];
    TLSOuterRecordBufferLast++;
    TempStart++;
    if( TempStart >= RawBufferLength )
      TempStart = 0; // Wrap it around.

    // Get the low byte of the length.
    // OR it with the high byte.
    OuterTLSLength |= RawBuffer[TempStart];
    TLSOuterRecordBuffer[TLSOuterRecordBufferLast] = RawBuffer[TempStart];
    TLSOuterRecordBufferLast++;
    TempStart++;
    if( TempStart >= RawBufferLength )
      TempStart = 0; // Wrap it around.

    // RFC says length must not exceed 2^14, which is 16,384.
    // But you might have to allow longer lengths (up to 64K) to be compatible
    // with some bad implementations that are out there.
    if( OuterTLSLength > MaximumTLSRecordLength )
      {
      StatusString += "Received an invalid message length in CustomerTLSClient.ReadNextOuterTLSRecord().\r\n";
      // To Do: Send an alert and gracefully shut it down.
      // "protocol_version" alert?
      FreeEverything();
      return false;
      }

    if( OuterTLSLength < 0 )
      {
      StatusString += "OuterTLSLength < 0. This is a bug in CustomerTLSClient.ReadNextOuterTLSRecord().\r\n";
      return false;
      }

    // if( OuterTLSLength < 1 )
    // Some messages, depending on the content type, aren't supposed to be empty.

    if( OuterTLSLength > HowManyInRawBuffer )
      {
      // Not a full message to process yet, so return false.
      StatusString += "Haven't received all the bytes yet.\r\n";
      return false;
      }

    if( TLSOuterRecordBufferLast != 5 )
      {
      StatusString += "TLSOuterRecordBufferLast != 5. This is a bug in CustomerTLSClient.ReadNextOuterTLSRecord().\r\n";
      return false;
      }

    // This is a limited form of a while( it's not at the end yet ).
    for( int Count = 0; Count < RawBufferLength; Count++ )
      {
      TLSOuterRecordBuffer[TLSOuterRecordBufferLast] = RawBuffer[TempStart];
      TLSOuterRecordBufferLast++;
      TempStart++;
      if( TempStart >= RawBufferLength )
        TempStart = 0; // Wrap it around.

      // It's the 5 initial header bytes plus the length it says it is.
      if( TLSOuterRecordBufferLast >= (OuterTLSLength + 5) )
        break; // It got past the end of the record.

      }

    // Move RawBufferStart to the new starting position for the next message.
    // If it never got this far then RawBufferStart hasn't changed and it would
    // start reading next time from the same beginning position.
    RawBufferStart = TempStart;

    // Since HowManyInRawBuffer was checked against OuterTLSLength, this can't
    // have RawBufferStart going past RawBufferEnd.  And OuterTLSLength is at
    // least a reasonable number.  But if there are no more messages that came
    // in then RawBufferStart would be equal to RawBufferEnd here. Otherwise
    // there's another message, or part of a message, still in the raw buffer.

    // There is now a new raw and unchecked message in the TLSOuterRecordBuffer.
    // It is totally unverified and it could have been sent by a hacker.
    // There is no telling what's in it at this point.
    StatusString += "OuterTLSLength is: " + OuterTLSLength.ToString() + "\r\n";
    StatusString += "\r\n";
    return true;

    // Some servers don't put the handshake messages in separate
    // outer TLS messages like some other servers do.
    // Some will combine these inner messages in to one outer message.
    // ServerHello
    // Certificate
    // ServerHelloDone

    }
    catch( Exception Except )
      {
      StatusString += "Exception in CustomerTLSClient.ReadNextOuterTLSRecord():\r\n";
      StatusString += Except.Message;
      FreeEverything();
      return false;
      }
    }



  private bool ProcessOneOuterTLSRecord()
    {
    try
    {
    string ShowS = "Received an outer message of type: ";
    if( OuterTLSContentType == 20 )
      {
      StatusString += ShowS + "ChangeCipherSpec\r\n";
      return true; // ProcessChangeCipherSpecMessage();
      }

    if( OuterTLSContentType == 21 )
      {
      StatusString += ShowS + "Alert\r\n";
      return true; // ProcessAlertMessage();
      }

    if( OuterTLSContentType == 22 )
      {
      StatusString += ShowS + "Handshake\r\n";
      return ProcessHandshakeMessages();
      }

    if( OuterTLSContentType == 23 )
      {
      StatusString += ShowS + "Application\r\n";
      return true; // ProcessApplicationMessage();
      }

    if( OuterTLSContentType == 24 )
      {
      StatusString += ShowS + "Heartbeat\r\n";
      }

    StatusString += "This is a bug. It should never happen since the content type was already checked.\r\n";
    StatusString += "OuterTLSContentType is: " + OuterTLSContentType.ToString() + "\r\n";
    FreeEverything();
    return false;
    }
    catch( Exception Except )
      {
      StatusString += "Exception in CustomerTLSClient.ProcessOneOuterTLSRecord():\r\n";
      StatusString += Except.Message + "\r\n";
      FreeEverything();
      return false;
      }
    }



  private bool ProcessHandshakeMessages()
    {
    int Index = 5; // After the TLS outer header.

    // There can be multiple inner messages of the same type in one outer TLS message.
    // While() there are still messages to process, but don't do it forever.
    for( int Count = 0; Count < 100; Count++ ) // There won't be 100 messages in it.
      {
      if( Index >= TLSOuterRecordBufferLast )
        {
        StatusString += "No more inner messages to process.\r\n";
        return true; // It's done.
        }

      int MessageType = TLSOuterRecordBuffer[Index];
      switch( MessageType )
        {
        case 0: // HelloRequest
          StatusString += "Received a HelloRequest.\r\n";
          return true;

        case 1: // ClientHello
          StatusString += "It should not be getting a ClientHello on this side.\r\n";
          return true;

        case 2: // ServerHello
          // Get the Index that's at the end of this message.
          Index = Handshake.ProcessServerHello( Index, TLSOuterRecordBuffer, TLSOuterRecordBufferLast );
          if( Index < 1 )
            return true; // Usually it's an error.

          break; // Go around the loop again, where it checks the Index at the top.

        case 4: // NewSessionTicket
          StatusString += "Received a NewSessionTicket.\r\n";
          return true;

        case 11: // Certificate
          Index = Handshake.ProcessX509Certificates( Index, TLSOuterRecordBuffer, TLSOuterRecordBufferLast );
          if( Index < 1 )
            return true; // Usually it's an error.

          break; // Go around the loop again, where it checks the Index at the top.

        case 12: // ServerKeyExchange
          StatusString += "Received a ServerKeyExchange.\r\n";
          return true;

        case 13: // CertificateRequest
          StatusString += "Received a CertificateRequest.\r\n";
          return true;

        case 14: // ServerHelloDone
          StatusString += "Received a ServerHelloDone.\r\n";
          return true;

        case 15: // CertificateVerify
          StatusString += "Received a CertificateVerify.\r\n";
          return true;

        case 16: // ClientKeyExchange
          StatusString += "Received a ClientKeyExchange.\r\n";
          return true;

        case 20: // Finished
          StatusString += "Received a Finished.\r\n";
          return true;

        case 21: // CertificateUrl
          StatusString += "Received a CertificateUrl.\r\n";
          return true;

        case 22: // CertificateStatus
          StatusString += "Received a CertificateStatus.\r\n";
          return true;

        default:
          StatusString += "It didn't find a matching handshake type.\r\n";
          return false;

        }
      }

    StatusString += "It went around 100 times and never exited correctly.";
    return false;
    }




  internal bool SendCrudeClientHello()
    {
    return Handshake.SendCrudeClientHello( CryptoRand, this );
    }



  }
}
*/



/*

// Programming by Eric Chauvin.
// Notes on this source code are at:
// ericbreakingrsa.blogspot.com


using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Security.Cryptography;

// http://www.x500standard.com/



namespace ExampleServer
{
  class ProcessHandshake
  {
  private string StatusString = "";
  private DomainX509Record X509Record;
  private string DomainName = "";


  private ProcessHandshake()
    {
    }


  internal ProcessHandshake( string UseDomainName )
    {
    DomainName = UseDomainName;
    }



  internal DomainX509Record GetX509Record()
    {
    return X509Record;
    }



  internal string GetStatusString()
    {
    string Result = StatusString;
    if( X509Record != null )
      Result += X509Record.GetStatusString();

    StatusString = "";
    return Result;
    }




  internal int ProcessX509Certificates( int Index, byte[] TLSOuterRecordBuffer, int TLSOuterRecordBufferLast )
    {
    StatusString += "Top of Certificate.\r\n";

    // This is a chain of certificates.
    int OriginalIndex = Index;
    StatusString += "\r\n";
    StatusString += "Processing Certificate message.\r\n";
    StatusString += "Index is: " + Index.ToString() + "\r\n";

    int MessageType = TLSOuterRecordBuffer[Index];
    Index++;
    if( MessageType != 11 )
      {
      StatusString += "This is a bug. MessageType != 11.\r\n";
      ShowBytesInBuffer( TLSOuterRecordBuffer, TLSOuterRecordBufferLast );
      return -1;
      }

    StatusString += "MessageType: " + MessageType.ToString() + "\r\n";

    int Length = TLSOuterRecordBuffer[Index];
    Length <<= 8;
    Index++;
    Length |= TLSOuterRecordBuffer[Index];
    Length <<= 8;
    Index++;
    Length |= TLSOuterRecordBuffer[Index];
    Index++;

    StatusString += "Length is: " + Length.ToString() + "\r\n";
    if( TLSOuterRecordBufferLast < (Index + Length) )
      {
      StatusString += "TLSOuterRecordBufferLast < (Index + Length). Length is: " + Length.ToString() + "\r\n";
      ShowBytesInBuffer( TLSOuterRecordBuffer, TLSOuterRecordBufferLast );
      return -1;
      }

    // "Implementations MUST NOT send zero-length fragments of Handshake,
    // Alert, or ChangeCipherSpec content types."
    if( Length < 1 )
      {
      StatusString += "The length of this message was less than one. Length is: " + Length.ToString() + "\r\n";
      ShowBytesInBuffer( TLSOuterRecordBuffer, TLSOuterRecordBufferLast );
      throw( new Exception( "Length less than one." ));
      // return -1;
      }

    // This MoveTo gets returned as the index for the start of the
    // next inner message.
    // The message type plus 3 bytes for the length is 4 bytes for the
    // header.  So add the 4 bytes here.
    int MoveTo = OriginalIndex + Length + 4;
    StatusString += "MoveTo is: " + MoveTo.ToString() + "\r\n";

    byte[] X509Buffer = new byte[Length];
    for( int Count = 0; Count < Length; Count++ )
      {
      X509Buffer[Count] = TLSOuterRecordBuffer[Index];
      Index++;
      }

    X509Record = new DomainX509Record( DomainName );
    X509Record.ParseAndAddOneCertificateList( X509Buffer );
    StatusString += X509Record.GetStatusString();

    StatusString += "Index after loop is: " + Index.ToString() + "\r\n";
    return -1;
    }



  private void ShowBytesInBuffer( byte[] Buffer, int Last )
    {
    StringBuilder SBuilder = new StringBuilder();
    for( int Count = 0; Count < Last; Count++ )
      {
      // 127 is the DEL character to Delete something.
      if( (Buffer[Count] >= 32) && (Buffer[Count] < 127))
        {
        SBuilder.Append( "ASCII: " + Char.ToString( (char)Buffer[Count] ) );
        }
      else
        {
        int ToShow = Buffer[Count];
        SBuilder.Append( "\r\nByte: " + ToShow.ToString() + "\r\n" );
        }
      }

    StatusString += SBuilder.ToString();
    }



  internal int ProcessServerHello( int Index, byte[] TLSOuterRecordBuffer, int TLSOuterRecordBufferLast )
    {
    try
    {
    int OriginalIndex = Index;
    StatusString += "\r\n";
    StatusString += "Processing ServerHello message.\r\n";
    StatusString += "Index is: " + Index.ToString() + "\r\n";

    // Check if it's at least a reasonable number.  How long is this?
    // It says in the RFC that "The server may return an empty session_id to
    // indicate that the session will not be cached and therefore cannot be
    // resumed."  So it could have a length of zero.
    // if( TLSOuterRecordBufferLast < What?) )
      // {
      // StatusString += "TLSOuterRecordBufferLast < (Index + the other parts) in ProcessHandshakeServerHello().";
      // return -1;
      // }

    int MessageType = TLSOuterRecordBuffer[Index];
    Index++;
    if( MessageType != 2 )
      {
      StatusString += "This is a bug. MessageType != 2.\r\n";
      return -1;
      }

    StatusString += "MessageType: " + MessageType.ToString() + "\r\n";

    // HandshakeMessageNumber++;

    int Length = TLSOuterRecordBuffer[Index];
    Length <<= 8;
    Index++;
    Length |= TLSOuterRecordBuffer[Index];
    Length <<= 8;
    Index++;
    Length |= TLSOuterRecordBuffer[Index];
    Index++;

    StatusString += "Length is: " + Length.ToString() + "\r\n";
    if( TLSOuterRecordBufferLast < (Index + Length) )
      {
      StatusString += "TLSOuterRecordBufferLast < (Index + Length). Length is: " + Length.ToString() + "\r\n";
      return -1;
      }

    // "Implementations MUST NOT send zero-length fragments of Handshake,
    // Alert, or ChangeCipherSpec content types."
    if( Length < 1 )
      return -1;

    // This MoveTo gets returned as the index for the start of the
    // next inner message.
    // The message type plus 3 bytes for the length is 4 bytes for the
    // header.  So add the 4 bytes here.
    int MoveTo = OriginalIndex + Length + 4;
    StatusString += "MoveTo is: " + MoveTo.ToString() + "\r\n";

    // The server says what version of TLS it wants to use.
    int VersionMajor = TLSOuterRecordBuffer[Index];
    Index++;
    int VersionMinor = TLSOuterRecordBuffer[Index];
    Index++;

    StatusString += "VersionMajor: " + VersionMajor.ToString() + "\r\n";
    StatusString += "VersionMinor: " + VersionMinor.ToString() + "\r\n";

    if( VersionMajor != 3 )
      {
      StatusString += "The server didn't respond with major version 3.\r\n";
      return -1;
      }

    if( VersionMinor != 3 )
      {
      // So negotiate a lower version.  Not too low though.  (Too weak.)
      StatusString += "The server can't do TLS 1.2. It didn't respond with version 3.3.\r\n";
      return -1;
      }

    // Unix time is the first four bytes, then the other 28 are random.
    // "Clocks are not required to be set correctly by the basic TLS protocol."
    byte[] RandomBytes = new byte[32];
    for( int Count = 0; Count < 32; Count++ )
      {
      RandomBytes[Count] = TLSOuterRecordBuffer[Index];
      Index++;
      }

    int SessionIDLength = TLSOuterRecordBuffer[Index];
    Index++;

    if( SessionIDLength > 32 )
      {
      StatusString += "SessionIDLength is > 32: " + SessionIDLength.ToString() + "\r\n";
      return -1;
      }

    StatusString += "SessionIDLength: " + SessionIDLength.ToString() + "\r\n";
    // byte[] SessionIDArray;
    if( SessionIDLength > 0 )
      {
      // Fill up a byte array with the Session ID.
      // SessionIDArray = new byte[SessionIDLength];

      // What if SessionIDLength wasn't checked here?  Where would the Index go?
      for( int Count = 0; Count < SessionIDLength; Count++ )
        {
        if( Index >= TLSOuterRecordBufferLast )
          {
          StatusString += "Index >= TLSOuterRecordBufferLast.\r\n";
          return -1;
          }

        // SessionIDArray[Count] = TLSOuterRecordBuffer[Index];
        Index++;
        }
      }

    // There is no CipherSuiteLength for the ServerHello.
    // It's just the two bytes for the value of the Cipher suite.

    int CipherSuiteHigh = TLSOuterRecordBuffer[Index];
    Index++;
    int CipherSuiteLow = TLSOuterRecordBuffer[Index];
    Index++;

    StatusString += "CipherSuite is: 0x" + CipherSuiteHigh.ToString( "X2" ) +
                    ", 0x" + CipherSuiteLow.ToString( "X2" ) + "\r\n";

    int CompressionMethod = TLSOuterRecordBuffer[Index];
    Index++;

    // That's it unless there are extensions.

    if( CompressionMethod != 0 )
      {
      StatusString += "The Compression Method is not null.\r\n";
      return -1;
      }

    StatusString += "Index: " + Index.ToString() + "\r\n";
    StatusString += "MoveTo: " + MoveTo.ToString() + "\r\n";

    if( Index > MoveTo )
      {
      StatusString += "Index > MoveTo. This can't be right.\r\n";
      return -1;
      }

    if( Index < MoveTo )
      {
      StatusString += "This ServerHello message has extensions.\r\n";
      }
    else
      {
      // Then it must be equal here.
      StatusString += "No extensions in this ServerHello message.\r\n";
      return MoveTo;
      }

    // Transport Layer Extensions
    // https://tools.ietf.org/html/rfc6066
    // https://www.iana.org/assignments/tls-extensiontype-values/tls-extensiontype-values.xml

    int ExtensionType = TLSOuterRecordBuffer[Index];
    Index++;
    ExtensionType <<= 8;
    ExtensionType |= TLSOuterRecordBuffer[Index];
    Index++;

    StatusString += "ExtensionType: " + ExtensionType.ToString() + "\r\n";
    StatusString += "ExtensionType hex: 0x" + ExtensionType.ToString( "X2" ) + "\r\n";

    StatusString += "Index at the end is: " + Index.ToString() + "\r\n";

    // Ignore the extension for now:
    return MoveTo;

    // server_name(0)
    // max_fragment_length(1)
    // client_certificate_url(2)
    // trusted_ca_keys(3)
    // truncated_hmac(4)
    // status_request(5)
    // Signature algorithm 13
    }
    catch( Exception Except )
      {
      StatusString += "Exception in CustomerTLSClient.ProcessHandshakeServerHello():\r\n";
      StatusString += Except.Message + "\r\n";
      return -1;
      }
    }



  internal bool SendCrudeClientHello( RNGCryptoServiceProvider CryptoRand, CustomerTLSClient CustomerTLS )
    {
    try
    {
    int LengthOfOuterMessage = 52 - 5;
    byte[] ToSendBuf = new byte[LengthOfOuterMessage + 5];

    // The first five bytes are the outer TLS record.
    ToSendBuf[0] = 22; // Content type is Handshake
    ToSendBuf[1] = 3;  // Version Major   TLS version 1.2 is version 3.3 (of SSL).
    ToSendBuf[2] = 3;  // Version Minor
    ToSendBuf[3] = (byte)(LengthOfOuterMessage >> 8);
    ToSendBuf[4] = (byte)(LengthOfOuterMessage);

    // Start of the inner ClientHello message.
    ToSendBuf[5] = 1; // Message type 1 is a  ClientHello message.

    int LengthOfClientHelloMessage = LengthOfOuterMessage - 4;
    ToSendBuf[6] = (byte)(LengthOfClientHelloMessage >> 16);
    ToSendBuf[7] = (byte)(LengthOfClientHelloMessage >> 8);
    ToSendBuf[8] = (byte)(LengthOfClientHelloMessage);
    ToSendBuf[9] = 3;   // Version Major
    ToSendBuf[10] = 3;  // Version Minor

    // This part for Unix time is not in TLS 1.3 because there's no point in having it.
    // But it is here in TLS 1.2.
    ECTime RightNow = new ECTime();
    RightNow.SetToNow();
    ulong UnixTime = RightNow.ToUnixTime();
    ToSendBuf[11] = (byte)(UnixTime >> 24);
    ToSendBuf[12] = (byte)(UnixTime >> 16);
    ToSendBuf[13] = (byte)(UnixTime >> 8);
    ToSendBuf[14] = (byte)(UnixTime);

    // These bytes have to be cryptographically random.
    // These are used later in generating the master secret and keys, etc.
    byte[] RandomBytes = new byte[28];
    CryptoRand.GetBytes( RandomBytes );

    int Index = 15;
    for( int Count = 0; Count < 28; Count++ )
      {
      ToSendBuf[Index] = RandomBytes[Count];
      Index++;
      }

    // Index is 15 + 28 = 43.
    // StatusString += "Index at the end of Random bytes is: " + Index.ToString() + "\r\n";
    // Index at the end of Random bytes is: 43

    ToSendBuf[43] = 0; // Session ID Length is zero. It's not resuming a session here.

    // A browser sends a lot more cipher suites than this so this is
    // usually a lot longer.  The algorithm you prefer to use should
    // be listed first.  So they are listed in order of preference.
    ToSendBuf[44] = 0; // Cipher Suites Length high byte
    ToSendBuf[45] = 4; // Cipher Suites Length
    ToSendBuf[46] = 0;
    ToSendBuf[47] = 0x35; // TLS_RSA_WITH_AES_256_CBC_SHA    = { 0x00,0x35 };
    ToSendBuf[48] = 0;
    ToSendBuf[49] = 0x3D; // TLS_RSA_WITH_AES_256_CBC_SHA256 = { 0x00,0x3D };
    ToSendBuf[50] = 1; // Compression Methods Length
    ToSendBuf[51] = 0; // Compression Method is null.

    // This message has no extensions, so that's all there is to send.
    return CustomerTLS.SendBuffer( ToSendBuf );

    }
    catch( Exception Except )
      {
      StatusString += "Exception in SendCrudeClientHello()\r\n";
      StatusString += Except.Message + "\r\n";
      return false;
      }
    }




  }
}



*/
