COMPONENT = gb.net.curl
include $(top_srcdir)/component.am

INCLUDES = @CURL_INC@

gblib_LTLIBRARIES = gb.net.curl.la

gb_net_curl_la_LIBADD = @CURL_LIB@
gb_net_curl_la_LDFLAGS = -module @LD_FLAGS@

gb_net_curl_la_SOURCES = \
	gbcurl.h gbcurl.c main.h main.c CCurl.h CCurl.c CHttpClient.h CHttpClient.c \
	CFtpClient.h CFtpClient.c CNet.h CNet.c CProxy.h CProxy.c



 
