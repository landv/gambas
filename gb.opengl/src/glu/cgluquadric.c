/***************************************************************************

	cgluquadric.c

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

#define __CGLUQUADRIC_C

#include "gb_common.h"
#include "cgluquadric.h"

CGLUQUADRIC *CGLUQUADRIC_create()
{
	return GB.New(GB.FindClass("GluQuadric"), NULL, NULL);
};

static int GluQuadric_check(void *_object)
{
	return QUADRIC == NULL;
}


BEGIN_METHOD_VOID(GluQuadric_new)

	THIS->quadric = gluNewQuadric();

END_METHOD

BEGIN_METHOD_VOID(GluQuadric_free)

	if (QUADRIC)
		gluDeleteQuadric(QUADRIC);

END_METHOD


GB_DESC GluQuadricDesc[] =
{
	GB_DECLARE("GluQuadric", sizeof(CGLUQUADRIC)),
	GB_HOOK_CHECK(GluQuadric_check),

	GB_METHOD("_new", NULL, GluQuadric_new, NULL),
	GB_METHOD("_free", NULL, GluQuadric_free, NULL),
	
	GB_END_DECLARE
};

