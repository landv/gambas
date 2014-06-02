/***************************************************************************

  systray.h

  (c) 2000-2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __SYSTRAY_H
#define __SYSTRAY_H

#include "config.h"
#include "../x11.h"
#include "../c_x11systray.h"
#include "../main.h"

typedef
	struct TrayIcon
	CX11SYSTRAYICON;

#if 0
/* enable debugging code */
#undef DEBUG

/* delay sending of embedding confirmation */
#undef DELAY_EMBEDDING_CONFIRMATION

/* use xprop/xwininfo to dump icon window info */
#undef ENABLE_DUMP_WIN_INFO

/* use non-portable hack to exit gracefuly on signal */
#undef ENABLE_GRACEFUL_EXIT_HACK

/* System has usable backtrace implementation. */
#undef HAVE_BACKTRACE

/* Define to 1 if you have the <ctype.h> header file. */
#undef HAVE_CTYPE_H

/* Define to 1 if you have the <errno.h> header file. */
#undef HAVE_ERRNO_H

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the <libgen.h> header file. */
#undef HAVE_LIBGEN_H

/* Define to 1 if you have the `Xpm' library (-lXpm). */
#undef HAVE_LIBXPM

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* System has usable printstack implementation. */
#undef HAVE_PRINTSTACK

/* Define to 1 if you have the <signal.h> header file. */
#undef HAVE_SIGNAL_H

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdio.h> header file. */
#undef HAVE_STDIO_H

/* Define to 1 if you have the <stdlib.h> header file. */
#undef HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#undef HAVE_STRING_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have the <time.h> header file. */
#undef HAVE_TIME_H

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* disable native kde tray icons support */
#undef NO_NATIVE_KDE

/* Name of package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* enable full event trace (debug) */
#undef TRACE_EVENTS

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# undef _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# undef _TANDEM_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# undef __EXTENSIONS__
#endif


/* Version number of package */
#undef VERSION

/* enable XPM background support */
#undef XPM_SUPPORTED

/* Define to 1 if the X Window System is missing or not being used. */
#undef X_DISPLAY_MISSING

/* Define to 1 if on MINIX. */
#undef _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#undef _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#undef _POSIX_SOURCE
#endif

//#define DEBUG

void SYSTRAY_init(Display *display, Window window);
void SYSTRAY_exit();
void SYSTRAY_event_filter(XEvent *ev);
int SYSTRAY_get_count();
CX11SYSTRAYICON *SYSTRAY_get(int i);
void SYSTRAY_refresh(void);

#endif
