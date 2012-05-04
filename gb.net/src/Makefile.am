COMPONENT = gb.net
include $(top_srcdir)/component.am

EXTRA_DIST = doc

gblib_LTLIBRARIES = gb.net.la

gb_net_la_LIBADD = @NET_LIB@
gb_net_la_LDFLAGS = -module @LD_FLAGS@ @NET_LDFLAGS@
gb_net_la_CPPFLAGS = @NET_INC@

gb_net_la_SOURCES = gb_network.h main.h main.c tools.h tools.c CDnsClient.h CDnsClient.c CSocket.h CSocket.c CServerSocket.h CServerSocket.c CUdpSocket.h CUdpSocket.c CSerialPort.h CSerialPort.c CNet.h CNet.c



 
