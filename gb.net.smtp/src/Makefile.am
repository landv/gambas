COMPONENT = gb.net.smtp
include $(top_srcdir)/component.am

INCLUDES = @SMTP_INC@

gblib_LTLIBRARIES = gb.net.smtp.la

gb_net_smtp_la_LIBADD = @SMTP_LIB@
gb_net_smtp_la_LDFLAGS = -module @LD_FLAGS@

gb_net_smtp_la_SOURCES = main.c main.h \
libsmtp.h libsmtp_comm.c libsmtp_connection.c libsmtp_data.c libsmtp_error.c \
libsmtp_mime.h base64.c comm.c parts.c quoted.c \
CSmtpClient.h CSmtpClient.c

