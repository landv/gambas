/***************************************************************************

  gshare.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GSHARE_H
#define __GSHARE_H

#include "gtag.h"

//#define DEBUG_SHARE 1

#ifdef DEBUG_SHARE

class gShare
{
public:
  gShare() { nref = 1; tag = NULL; fprintf(stderr, "gShare: %p (1)\n", this); }
  virtual ~gShare() { fprintf(stderr, "~gShare: %p\n", this); if (tag) while (nref > 1) nref--, tag->unref(); }
  
  void ref() { nref++; if (tag) tag->ref(); fprintf(stderr, "gShare::ref: %p (%d)\n", this, nref); }
  void unref() { nref--; fprintf(stderr, "gShare::unref: %p (%d)\n", this, nref); if (nref <= 0) delete this; else if (tag) tag->unref(); }
  int refCount() { return nref; }
  
  void setTag(gTag *t) { tag = t; for (int i = 0; i < (nref - 1); i++) tag->ref(); }
  gTag *getTag() { return tag; }
  void *getTagValue() { return tag->get(); }
  
protected:
  static void assign(gShare **dst, gShare *src = 0)
  {
    if (src) src->ref();
    if (*dst) (*dst)->unref();
    *dst = src;
  }
  
private:
  int nref;
  gTag *tag;
};

#else

class gShare
{
public:
  gShare() { nref = 1; tag = NULL; }
  virtual ~gShare() { if (tag) { while (nref > 1) nref--, tag->unref(); delete tag; } }
  
  void ref() { nref++; if (tag) tag->ref(); }
  void unref() { nref--; if (nref <= 0) delete this; else if (tag) tag->unref(); }
  int refCount() { return nref; }
  
  void setTag(gTag *t) { tag = t; for (int i = 0; i < (nref - 1); i++) tag->ref(); }
  gTag *getTag() { return tag; }
  void *getTagValue() { return tag->get(); }
  
protected:
  static void assign(gShare **dst, gShare *src = 0)
  {
    if (src) src->ref();
    if (*dst) (*dst)->unref();
    *dst = src;
  }
  
private:
  int nref;
  gTag *tag;
};

#endif

#endif

