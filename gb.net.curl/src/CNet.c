/***************************************************************************

  CNet.c

  Advanced Network component

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CNET_C
#include "main.h"
#include <stdio.h>
#include <curl/curl.h>

#include "CNet.h"
#define CURLGB(x) (-1)*(1000+x)

/***************************************************************
 Here we declare the public interface of NetCode class
 ***************************************************************/
GB_DESC CNetDesc[] =
{

  GB_DECLARE("Net", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT ("Asynchronous","i",0),
  GB_CONSTANT ("Synchronous","i",1),
  /* net-curl proxies */
  GB_CONSTANT ("ProxyHTTP","i",CURLPROXY_HTTP),
  GB_CONSTANT ("ProxySocks5","i",CURLPROXY_SOCKS5),
  /* net-curl autohorization */
  #ifdef CURLAUTH_NONE
  GB_CONSTANT ("AuthNone","i",CURLAUTH_NONE),
  GB_CONSTANT ("AuthBasic","i",CURLAUTH_BASIC),
  GB_CONSTANT ("AuthNTLM","i",CURLAUTH_NTLM),
  GB_CONSTANT ("AuthDIGEST","i",CURLAUTH_DIGEST),
  GB_CONSTANT ("AuthGSSNEGOTIATE","i",CURLAUTH_GSSNEGOTIATE),
  #else
  GB_CONSTANT ("AuthNone","i",0),
  GB_CONSTANT ("AuthBasic","i",1),
  GB_CONSTANT ("AuthNTLM","i",2),
  GB_CONSTANT ("AuthDIGEST","i",3),
  GB_CONSTANT ("AuthGSSNEGOTIATE","i",4),
  #endif
  /* net-curl error codes */
  GB_CONSTANT ("UnsupportedProtocol","i",CURLGB(CURLE_UNSUPPORTED_PROTOCOL)),
  GB_CONSTANT ("InitFailed","i",CURLGB(CURLE_FAILED_INIT)),
  GB_CONSTANT ("URLMalformat","i",CURLGB(CURLE_URL_MALFORMAT)),
  GB_CONSTANT ("URLMalformatUser","i",CURLGB(CURLE_URL_MALFORMAT_USER)),
  GB_CONSTANT ("UnableToResolveProxy","i",CURLGB(CURLE_COULDNT_RESOLVE_PROXY)),
  GB_CONSTANT ("UnableToResolveHost","i",CURLGB(CURLE_COULDNT_RESOLVE_HOST)),
  GB_CONSTANT ("UnableToConnect","i",CURLGB(CURLE_COULDNT_CONNECT)),
  GB_CONSTANT ("FTPWeirdServerReply","i",CURLGB(CURLE_FTP_WEIRD_SERVER_REPLY)),
  GB_CONSTANT ("FTPAccessDenied","i",CURLGB(CURLE_FTP_ACCESS_DENIED)),
  GB_CONSTANT ("FTPUserIncorrect","i",CURLGB(CURLE_FTP_USER_PASSWORD_INCORRECT)),
  GB_CONSTANT ("FTPWeirdPassReply","i",CURLGB(CURLE_FTP_WEIRD_PASS_REPLY)),
  GB_CONSTANT ("FTPWeirdUserReply","i",CURLGB(CURLE_FTP_WEIRD_USER_REPLY)),
  GB_CONSTANT ("FTPWeirdPasvReply","i",CURLGB(CURLE_FTP_WEIRD_PASV_REPLY)),
  GB_CONSTANT ("FTPWeird227Format","i",CURLGB(CURLE_FTP_WEIRD_227_FORMAT)),
  GB_CONSTANT ("UnableToGetHost","i",CURLGB(CURLE_FTP_CANT_GET_HOST)),
  GB_CONSTANT ("UnableToReconnect","i",CURLGB(CURLE_FTP_CANT_RECONNECT)),
  GB_CONSTANT ("FTPUnableToSetBinary","i",CURLGB(CURLE_FTP_COULDNT_SET_BINARY)),
  GB_CONSTANT ("PartialFile","i",CURLGB(CURLE_PARTIAL_FILE)),
  GB_CONSTANT ("FTPUnableToRETRFile","i",CURLGB(CURLE_FTP_COULDNT_RETR_FILE)),
  GB_CONSTANT ("FTPWriteError","i",CURLGB(CURLE_FTP_WRITE_ERROR)),
  GB_CONSTANT ("FTPQuoteError","i",CURLGB(CURLE_FTP_QUOTE_ERROR)),
  GB_CONSTANT ("HTTPReturnedError","i",CURLGB(CURLE_HTTP_RETURNED_ERROR)),
  GB_CONSTANT ("WriteError","i",CURLGB(CURLE_WRITE_ERROR)),
  GB_CONSTANT ("MalformatUser","i",CURLGB(CURLE_MALFORMAT_USER)),
  GB_CONSTANT ("UploadFailed","i",CURLGB(CURLE_FTP_COULDNT_STOR_FILE)),
  GB_CONSTANT ("ReadError","i",CURLGB(CURLE_READ_ERROR)),
  GB_CONSTANT ("OutOfMemory","i",CURLGB(CURLE_OUT_OF_MEMORY)),
  GB_CONSTANT ("OperationTimeout","i",CURLGB(CURLE_OPERATION_TIMEOUTED)),
  GB_CONSTANT ("FTPUnableToSetASCII","i",CURLGB(CURLE_FTP_COULDNT_SET_ASCII)),
  GB_CONSTANT ("FTPPortFailed","i",CURLGB(CURLE_FTP_PORT_FAILED)),
  GB_CONSTANT ("FTPUnableToUseRest","i",CURLGB(CURLE_FTP_COULDNT_USE_REST)),
  GB_CONSTANT ("FTPUnableToGetSize","i",CURLGB(CURLE_FTP_COULDNT_GET_SIZE)),
  GB_CONSTANT ("HTTPRangeError","i",CURLGB(CURLE_HTTP_RANGE_ERROR)),
  GB_CONSTANT ("HTTPPostError","i",CURLGB(CURLE_HTTP_POST_ERROR)),
  GB_CONSTANT ("SSLConnectError","i",CURLGB(CURLE_SSL_CONNECT_ERROR)),
  GB_CONSTANT ("BadDownloadResume","i",CURLGB(CURLE_BAD_DOWNLOAD_RESUME)),
  GB_CONSTANT ("UnableToReadFile","i",CURLGB(CURLE_FILE_COULDNT_READ_FILE)),
  GB_CONSTANT ("LDAPCannotBind","i",CURLGB(CURLE_LDAP_CANNOT_BIND)),
  GB_CONSTANT ("LDAPSearchFailed","i",CURLGB(CURLE_LDAP_SEARCH_FAILED)),
  GB_CONSTANT ("LibraryNotFound","i",CURLGB(CURLE_LIBRARY_NOT_FOUND)),
  GB_CONSTANT ("FunctionNotFound","i",CURLGB(CURLE_FUNCTION_NOT_FOUND)),
  GB_CONSTANT ("AbortedByCallback","i",CURLGB(CURLE_ABORTED_BY_CALLBACK)),
  GB_CONSTANT ("BadFunctionArgument","i",CURLGB(CURLE_BAD_FUNCTION_ARGUMENT)),
  GB_CONSTANT ("BadCallingOrder","i",CURLGB(CURLE_BAD_CALLING_ORDER)),
  GB_CONSTANT ("HTTPPortFailed","i",CURLGB(CURLE_HTTP_PORT_FAILED)),
  GB_CONSTANT ("BadPasswordEntered","i",CURLGB(CURLE_BAD_PASSWORD_ENTERED)),
  GB_CONSTANT ("TooManyRedirects","i",CURLGB(CURLE_TOO_MANY_REDIRECTS)) ,
  GB_CONSTANT ("TelnetUnknownOption","i",CURLGB(CURLE_UNKNOWN_TELNET_OPTION)),
  GB_CONSTANT ("TelnetBadSyntax","i",CURLGB(CURLE_TELNET_OPTION_SYNTAX)) ,
  GB_CONSTANT ("Obsolete","i",CURLGB(CURLE_OBSOLETE)),
  GB_CONSTANT ("SSLPeerCertificate","i",CURLGB(CURLE_SSL_PEER_CERTIFICATE)),
  GB_CONSTANT ("GotNothing","i",CURLGB(CURLE_GOT_NOTHING)),
  GB_CONSTANT ("SSLEngineNotFound","i",CURLGB(CURLE_SSL_ENGINE_NOTFOUND)),
  GB_CONSTANT ("SSLEngineSetFailed","i",CURLGB(CURLE_SSL_ENGINE_SETFAILED)),
  GB_CONSTANT ("SendError","i",CURLGB(CURLE_SEND_ERROR)),
  GB_CONSTANT ("RecvError","i",CURLGB(CURLE_RECV_ERROR)),
  GB_CONSTANT ("ShareInUse","i",CURLGB(CURLE_SHARE_IN_USE)),
  GB_CONSTANT ("SSLCertProblem","i",CURLGB(CURLE_SSL_CERTPROBLEM)),
  GB_CONSTANT ("SSLCipher","i",CURLGB(CURLE_SSL_CIPHER)),
  GB_CONSTANT ("SSLCacert","i",CURLGB(CURLE_SSL_CACERT)),
  GB_CONSTANT ("BadContentEncoding","i",CURLGB(CURLE_BAD_CONTENT_ENCODING)),
  #ifdef CURLE_LDAP_INVALID_URL // added in version 7.10.8
  GB_CONSTANT ("LDAPInvalidURL","i",CURLGB(CURLE_LDAP_INVALID_URL)),
  GB_CONSTANT ("FileSizeExceeded","i",CURLGB(CURLE_FILESIZE_EXCEEDED)),
  #endif


  GB_END_DECLARE
};




