/***************************************************************************

  CNet.c

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CNET_C
#include "main.h"
#include <stdio.h>
#include <curl/curl.h>

#include "CNet.h"

#define GBCURL(x) (-(1000+x))

// Constants not defined in old versions of libcurl

#ifndef CURLE_FTP_PRET_FAILED
#define CURLE_FTP_PRET_FAILED 84
#endif
#ifndef CURLE_FTP_BAD_FILE_LIST
#define CURLE_FTP_BAD_FILE_LIST 87
#endif
#ifndef CURLE_CHUNK_FAILED
#define CURLE_CHUNK_FAILED 88
#endif

#ifndef CURLAUTH_NONE
#define CURLAUTH_NONE         ((unsigned long)0)       /* nothing */
#define CURLAUTH_BASIC        (((unsigned long)1)<<0)  /* Basic (default) */
#define CURLAUTH_DIGEST       (((unsigned long)1)<<1)  /* Digest */
#define CURLAUTH_GSSNEGOTIATE (((unsigned long)1)<<2)  /* GSS-Negotiate */
#define CURLAUTH_NTLM         (((unsigned long)1)<<3)  /* NTLM */
#define CURLAUTH_ANY ~0                                /* all types set */
#define CURLAUTH_ANYSAFE (~CURLAUTH_BASIC)
#endif

#ifndef CURLAUTH_DIGEST_IE
#define CURLAUTH_DIGEST_IE    (((unsigned long)1)<<4)
#undef CURLAUTH_ANY
#define CURLAUTH_ANY          (~CURLAUTH_DIGEST_IE)
#undef CURLAUTH_ANYSAGE
#define CURLAUTH_ANYSAFE      (~(CURLAUTH_BASIC|CURLAUTH_DIGEST_IE))
#endif

#ifndef CURLAUTH_NTLM_WB
#define CURLAUTH_NTLM_WB      (((unsigned long)1)<<5)
#endif

#if LIBCURL_VERSION_NUM < 0x071202
#define CURLE_AGAIN 81
#endif

#if LIBCURL_VERSION_NUM < 0x071300
#define CURLE_SSL_CRL_BADFILE 82
#define CURLE_SSL_ISSUER_ERROR 83
#define CURLE_FTP_PRET_FAILED 84
#define CURLE_RTSP_CSEQ_ERROR 85
#define CURLE_RTSP_SESSION_ERROR 86
#define CURLE_FTP_BAD_FILE_LIST 87
#define CURLE_CHUNK_FAILED 88
#endif

#if LIBCURL_VERSION_NUM < 0x071505
#define CURLE_NOT_BUILT_IN 4
#endif

#if LIBCURL_VERSION_NUM < 0x071800
#define CURLE_FTP_ACCEPT_FAILED 10
#define CURLE_FTP_ACCEPT_TIMEOUT 12
#endif

GB_DESC CNetDesc[] =
{
	GB_DECLARE("Net", 0), GB_VIRTUAL_CLASS(),

	// Net states used by curl
	//GB_CONSTANT("Inactive", "i", 0),
	//GB_CONSTANT("ReceivingData","i",4),
	//GB_CONSTANT("Connecting", "i", 6),

	GB_CONSTANT ("Synchronous", "i", 0),
	GB_CONSTANT ("Asynchronous", "i", 1),
	/* net-curl proxies */
	GB_CONSTANT ("ProxyHTTP", "i", CURLPROXY_HTTP),
	GB_CONSTANT ("ProxySocks5", "i", CURLPROXY_SOCKS5),
	/* net-curl autohorization */
	GB_CONSTANT("AuthNone", "i", CURLAUTH_NONE),
	GB_CONSTANT("AuthBasic", "i", CURLAUTH_BASIC),
	GB_CONSTANT("AuthNtlm", "i", CURLAUTH_NTLM),
	GB_CONSTANT("AuthDigest", "i", CURLAUTH_DIGEST),
	GB_CONSTANT("AuthDigestIE", "i", CURLAUTH_DIGEST_IE),
	GB_CONSTANT("AuthNtlmWb", "i", CURLAUTH_NTLM_WB),
	GB_CONSTANT("AuthGssNegotiate", "i", CURLAUTH_GSSNEGOTIATE),
	GB_CONSTANT("AuthAny", "i", CURLAUTH_ANY),
	GB_CONSTANT("AuthAnySafe", "i", CURLAUTH_ANYSAFE),

	GB_CONSTANT("UnsupportedProtocol", "i", GBCURL(CURLE_UNSUPPORTED_PROTOCOL)),
	GB_CONSTANT("FailedInit", "i", GBCURL(CURLE_FAILED_INIT)),
	GB_CONSTANT("URLMalformat", "i", GBCURL(CURLE_URL_MALFORMAT)),
	GB_CONSTANT("UnableToResolveProxy", "i", GBCURL(CURLE_COULDNT_RESOLVE_PROXY)),
	GB_CONSTANT("UnableToResolveHost", "i", GBCURL(CURLE_COULDNT_RESOLVE_HOST)),
	GB_CONSTANT("UnableToConnect", "i", GBCURL(CURLE_COULDNT_CONNECT)),
	GB_CONSTANT("FTPWeirdServerReply", "i", GBCURL(CURLE_FTP_WEIRD_SERVER_REPLY)),
	GB_CONSTANT("RemoteAccessDenied", "i", GBCURL(CURLE_REMOTE_ACCESS_DENIED)),
	GB_CONSTANT("FTPWeirdPassReply", "i", GBCURL(CURLE_FTP_WEIRD_PASS_REPLY)),
	GB_CONSTANT("FTPWeirdPasvReply", "i", GBCURL(CURLE_FTP_WEIRD_PASV_REPLY)),
	GB_CONSTANT("FTPWeird227Format", "i", GBCURL(CURLE_FTP_WEIRD_227_FORMAT)),
	GB_CONSTANT("FTPUnableToGetHost", "i", GBCURL(CURLE_FTP_CANT_GET_HOST)),
	GB_CONSTANT("FTPUnableToSetType", "i", GBCURL(CURLE_FTP_COULDNT_SET_TYPE)),
	GB_CONSTANT("PartialFile", "i", GBCURL(CURLE_PARTIAL_FILE)),
	GB_CONSTANT("FTPUnableToRETRFile", "i", GBCURL(CURLE_FTP_COULDNT_RETR_FILE)),
	GB_CONSTANT("QuoteError", "i", GBCURL(CURLE_QUOTE_ERROR)),
	GB_CONSTANT("HttpReturnedError", "i", GBCURL(CURLE_HTTP_RETURNED_ERROR)),
	GB_CONSTANT("WriteError", "i", GBCURL(CURLE_WRITE_ERROR)),
	GB_CONSTANT("UploadFailed", "i", GBCURL(CURLE_UPLOAD_FAILED)),
	GB_CONSTANT("ReadError", "i", GBCURL(CURLE_READ_ERROR)),
	GB_CONSTANT("OutOfMemory", "i", GBCURL(CURLE_OUT_OF_MEMORY)),
	GB_CONSTANT("OperationTimeout", "i", GBCURL(CURLE_OPERATION_TIMEDOUT)),
	GB_CONSTANT("FTPPortFailed", "i", GBCURL(CURLE_FTP_PORT_FAILED)),
	GB_CONSTANT("FTPUnableToUseRest", "i", GBCURL(CURLE_FTP_COULDNT_USE_REST)),
	GB_CONSTANT("RangeError", "i", GBCURL(CURLE_RANGE_ERROR)),
	GB_CONSTANT("HTTPPostError", "i", GBCURL(CURLE_HTTP_POST_ERROR)),
	GB_CONSTANT("SSLConnectError", "i", GBCURL(CURLE_SSL_CONNECT_ERROR)),
	GB_CONSTANT("BadDownloadResume", "i", GBCURL(CURLE_BAD_DOWNLOAD_RESUME)),
	//GB_CONSTANT("FileUnableToReadFile", "i", GBCURL(CURLE_FILE_COULDNT_READ_FILE)),
	//GB_CONSTANT("LDAPCannotBind", "i", GBCURL(CURLE_LDAP_CANNOT_BIND)),
	//GB_CONSTANT("LDAPSearchFailed", "i", GBCURL(CURLE_LDAP_SEARCH_FAILED)),
	GB_CONSTANT("FunctionNotFound", "i", GBCURL(CURLE_FUNCTION_NOT_FOUND)),
	GB_CONSTANT("AbortedByCallback", "i", GBCURL(CURLE_ABORTED_BY_CALLBACK)),
	GB_CONSTANT("BadFunctionArgument", "i", GBCURL(CURLE_BAD_FUNCTION_ARGUMENT)),
	GB_CONSTANT("InterfaceFailed", "i", GBCURL(CURLE_INTERFACE_FAILED)),
	GB_CONSTANT("TooManyRedirects", "i", GBCURL(CURLE_TOO_MANY_REDIRECTS )),
	//GB_CONSTANT("UnknownTelnetOption", "i", GBCURL(CURLE_UNKNOWN_TELNET_OPTION)),
	//GB_CONSTANT("TelnetOptionSyntax", "i", GBCURL(CURLE_TELNET_OPTION_SYNTAX)),
	GB_CONSTANT("PeerFailedVerification", "i", GBCURL(CURLE_PEER_FAILED_VERIFICATION)),
	GB_CONSTANT("GotNothing", "i", GBCURL(CURLE_GOT_NOTHING)),
	GB_CONSTANT("SSLEngineNotFound", "i", GBCURL(CURLE_SSL_ENGINE_NOTFOUND)),
	GB_CONSTANT("SSLEngineSetFailed", "i", GBCURL(CURLE_SSL_ENGINE_SETFAILED)),
	GB_CONSTANT("SendError", "i", GBCURL(CURLE_SEND_ERROR)),
	GB_CONSTANT("RecvError", "i", GBCURL(CURLE_RECV_ERROR)),
	GB_CONSTANT("SSLCertProblem", "i", GBCURL(CURLE_SSL_CERTPROBLEM)),
	GB_CONSTANT("SSLCipher", "i", GBCURL(CURLE_SSL_CIPHER)),
	GB_CONSTANT("SSLCacert", "i", GBCURL(CURLE_SSL_CACERT)),
	GB_CONSTANT("BadContentEncoding", "i", GBCURL(CURLE_BAD_CONTENT_ENCODING)),
	//GB_CONSTANT("LDAPInvalidURL", "i", GBCURL(CURLE_LDAP_INVALID_URL)),
	GB_CONSTANT("FileSizeExceeded", "i", GBCURL(CURLE_FILESIZE_EXCEEDED)),
	//GB_CONSTANT("UseSSLFailed", "i", GBCURL(CURLE_USE_SSL_FAILED)),
	GB_CONSTANT("SendFailRewind", "i", GBCURL(CURLE_SEND_FAIL_REWIND)),
	GB_CONSTANT("SSLEngineInitFailed", "i", GBCURL(CURLE_SSL_ENGINE_INITFAILED)),
	GB_CONSTANT("LoginDenied", "i", GBCURL(CURLE_LOGIN_DENIED)),
	//GB_CONSTANT("TFTPNotFound", "i", GBCURL(CURLE_TFTP_NOTFOUND)),
	//GB_CONSTANT("TFTPPerm", "i", GBCURL(CURLE_TFTP_PERM)),
	GB_CONSTANT("RemoteDiskFull", "i", GBCURL(CURLE_REMOTE_DISK_FULL)),
	//GB_CONSTANT("TFTPIllegal", "i", GBCURL(CURLE_TFTP_ILLEGAL)),
	//GB_CONSTANT("TFTPUnknownID", "i", GBCURL(CURLE_TFTP_UNKNOWNID)),
	GB_CONSTANT("RemoteFileExists", "i", GBCURL(CURLE_REMOTE_FILE_EXISTS)),
	//GB_CONSTANT("TFTPNoSuchUser", "i", GBCURL(CURLE_TFTP_NOSUCHUSER)),
	GB_CONSTANT("ConvFailed", "i", GBCURL(CURLE_CONV_FAILED)),
	GB_CONSTANT("ConvRequired", "i", GBCURL(CURLE_CONV_REQD)),
	GB_CONSTANT("SSLCacertBadFile", "i", GBCURL(CURLE_SSL_CACERT_BADFILE)),
	GB_CONSTANT("RemoteFileNotFound", "i", GBCURL(CURLE_REMOTE_FILE_NOT_FOUND)),
	//GB_CONSTANT("SSH", "i", GBCURL(CURLE_SSH)),
	GB_CONSTANT("SSLShutdownFailed", "i", GBCURL(CURLE_SSL_SHUTDOWN_FAILED)),
	//GB_CONSTANT("Again", "i", GBCURL(CURLE_AGAIN)),
	GB_CONSTANT("SSLCrlBadfile", "i", GBCURL(CURLE_SSL_CRL_BADFILE)),
	GB_CONSTANT("SSLIssuerError", "i", GBCURL(CURLE_SSL_ISSUER_ERROR)),
	GB_CONSTANT("FTPPretFailed", "i", GBCURL(CURLE_FTP_PRET_FAILED)),
	//GB_CONSTANT("RTSPCSeqError", "i", GBCURL(CURLE_RTSP_CSEQ_ERROR)),
	//GB_CONSTANT("RTSPSessionError", "i", GBCURL(CURLE_RTSP_SESSION_ERROR)),
	GB_CONSTANT("FTPBadFileList", "i", GBCURL(CURLE_FTP_BAD_FILE_LIST)),
	GB_CONSTANT("ChunkFailed", "i", GBCURL(CURLE_CHUNK_FAILED)),

	GB_END_DECLARE
};
