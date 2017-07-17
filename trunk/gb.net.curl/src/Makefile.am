COMPONENT = gb.net.curl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.net.curl.la

gb_net_curl_la_LIBADD = @THREAD_LIB@ @CURL_LIB@
gb_net_curl_la_LDFLAGS = -module @LD_FLAGS@ @CURL_LDFLAGS@
gb_net_curl_la_CPPFLAGS = @THREAD_INC@ @CURL_INC@

gb_net_curl_la_SOURCES = \
	gbcurl.h gbcurl.c \
	main.h main.c \
	CCurl.h CCurl.c \
	CHttpClient.h CHttpClient.c \
	CFtpClient.h CFtpClient.c \
	CNet.h CNet.c \
	CProxy.h CProxy.c



 
