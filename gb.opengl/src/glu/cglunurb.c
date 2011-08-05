/***************************************************************************

	cglunurb.c

	(c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>
	(and Tomek Kolodziejczyk)

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

#define __CGLUNURB_C

#include "gb_common.h"
#include "cglunurb.h"

CGLUNURB *CGLUNURB_create(GLUnurbs *nurb)
{
	CGLUNURB *ob = GB.New(GB.FindClass("GluNurb"), NULL, NULL);
	ob->nurb = nurb;
	return ob;
}

GB_DESC GluNurbsDesc[] =
{
	GB_DECLARE("GluNurb", sizeof(CGLUNURB)),
	GB_NOT_CREATABLE(),
	GB_END_DECLARE
};

/*BEGIN_METHOD(CGLUQUADRIC_free, GB_OBJECT quadric)

	if (!quadric) 
		return;

	gluDeleteNurbsRenderer (quadric);

END_METHOD*/
