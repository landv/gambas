/***************************************************************************

  main.h

  gb.httpd component

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

#include "gb_common.h"
#include "gambas.h"

#ifndef __MAIN_C
extern const GB_INTERFACE *GB_PTR;
#endif

#define GB (*GB_PTR)

#define	LOG_EMERG	0							/* system is unusable */
#define	LOG_ALERT	1							/* action must be taken immediately */
#define	LOG_CRIT	2							/* critical conditions */
#define	LOG_ERR		3							/* error conditions */
#define	LOG_WARNING	4						/* warning conditions */
#define	LOG_NOTICE	5						/* normal but significant condition */
#define	LOG_INFO	6							/* informational */
#define	LOG_DEBUG	7							/* debug-level messages */

void syslog(int priority, const char *format, ...);
#define closelog()

void run_cgi();

#endif /* __MAIN_H */
