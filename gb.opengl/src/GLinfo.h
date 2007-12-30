/***************************************************************************

  GLinfo.h

  openGL component

  (c) 2005 Laurent Carlier <lordheavy@users.sourceforge.net>
           Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GLINFO_H
#define __GLINFO_H

#include "gambas.h"
#include "main.h"

DECLARE_METHOD(GLGETACCUMALPHABITS);
DECLARE_METHOD(GLGETACCUMCLEARVALUE);
DECLARE_METHOD(GLGETACCUMBLUEBITS);
DECLARE_METHOD(GLGETACCUMGREENBITS);
DECLARE_METHOD(GLGETACCUMREDBITS);
DECLARE_METHOD(GLGETALPHABITS);
DECLARE_METHOD(GLGETALPHATESTFUNC);
DECLARE_METHOD(GLGETALPHATESTREF);
DECLARE_METHOD(GLGETBLENDDST);
DECLARE_METHOD(GLGETBLENDSRC);
DECLARE_METHOD(GLGETCOLORMATERIALFACE);
DECLARE_METHOD(GLGETCOLORMATERIALPARAMETER);
DECLARE_METHOD(GLGETCULLFACEMODE);
DECLARE_METHOD(GLGETDEPTHBITS);
DECLARE_METHOD(GLGETDEPTHCLEARVALUE);
DECLARE_METHOD(GLGETDEPTHFUNC);
DECLARE_METHOD(GLGETDEPTHRANGE);
DECLARE_METHOD(GLGETDEPTHWRITEMASK);
DECLARE_METHOD(GLGETEDGEFLAG);
DECLARE_METHOD(GLGETFRONTFACE);
DECLARE_METHOD(GLGETFOGCOLOR);
DECLARE_METHOD(GLGETFOGDENSITY);
DECLARE_METHOD(GLGETFOGEND);
DECLARE_METHOD(GLGETFOGHINT);
DECLARE_METHOD(GLGETFOGINDEX);
DECLARE_METHOD(GLGETFOGMODE);
DECLARE_METHOD(GLGETFOGSTART);
DECLARE_METHOD(GLGETLIGHTMODELAMBIENT);
DECLARE_METHOD(GLGETLIGHTMODELLOCALVIEWER);
DECLARE_METHOD(GLGETLIGHTMODELTWOSIDE);
DECLARE_METHOD(GLGETLINESMOOTHHINT);
DECLARE_METHOD(GLGETLINESTIPPLEPATTERN);
DECLARE_METHOD(GLGETLINESTIPPLEREPEAT);
DECLARE_METHOD(GLGETLINEWIDTH);
DECLARE_METHOD(GLGETLINEWIDTHGRANULARITY);
DECLARE_METHOD(GLGETLINEWIDTHRANGE);
DECLARE_METHOD(GLGETLISTBASE);
DECLARE_METHOD(GLGETLISTINDEX);
DECLARE_METHOD(GLGETLISTMODE);
DECLARE_METHOD(GLGETMATRIXMODE);
DECLARE_METHOD(GLGETPERSPECTIVECORRECTIONHINT);
DECLARE_METHOD(GLGETPOINTSMOOTHHINT);
DECLARE_METHOD(GLGETPOINTSIZE);
DECLARE_METHOD(GLGETPOINTSIZEGRANULARITY);
DECLARE_METHOD(GLGETPOINTSIZERANGE);
DECLARE_METHOD(GLGETPOLYGONMODE);
DECLARE_METHOD(GLGETPOLYGONSMOOTHHINT);
DECLARE_METHOD(GLGETSCISSORBOX);
DECLARE_METHOD(GLGETSHADEMODEL);

DECLARE_METHOD(GLMAXLISTNESTING);
DECLARE_METHOD(GLMAXATTRIBSTACKDEPTH);
DECLARE_METHOD(GLMAXMODELVIEWSTACKDEPTH);
DECLARE_METHOD(GLMAXNAMESTACKDEPTH);
DECLARE_METHOD(GLMAXPROJECTIONSTACKDEPTH);
DECLARE_METHOD(GLMAXTEXTURESTACKDEPTH);
DECLARE_METHOD(GLMAXEVALORDER);
DECLARE_METHOD(GLMAXLIGHTS);
DECLARE_METHOD(GLMAXCLIPPLANES);
DECLARE_METHOD(GLMAXTEXTURESIZE);
//DECLARE_METHOD(GLMAXPIXELMAPTABLE);
DECLARE_METHOD(GLMAXVIEWPORTDIMS);
DECLARE_METHOD(GLMAXCLIENTATTRIBSTACKDEPTH);

#endif /* __GLINFO_H */
