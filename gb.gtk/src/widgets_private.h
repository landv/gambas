/***************************************************************************

  widgets_private.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __WIDGETS_PRIVATE_H
#define __WIDGETS_PRIVATE_H

#include <gtk/gtk.h>

void gMnemonic_correctText(char *st,char **buf);
guint gMnemonic_correctMarkup(char *st,char **buf);
void gMnemonic_returnText(char *st,char **buf);

/* Tools */
gPicture *Grab_gdkWindow(GdkWindow *win);

int gDialog_run(GtkDialog *wid);

#endif
