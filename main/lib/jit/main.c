/***************************************************************************

	main.c

	(c) 2000-2018 Benoît Minisini <g4mba5@gmail.com>

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

#define __MAIN_C

#include "jit.h"
#include "main.h"

GB_INTERFACE *GB_PTR EXPORT;
JIT_INTERFACE *JIT_PTR EXPORT;


BEGIN_METHOD(Jit_Translate, GB_STRING klass; GB_STRING from)

	GB.ReturnString(JIT_translate(GB.ToZeroString(ARG(klass)), GB.ToZeroString(ARG(from))));
	
END_METHOD


GB_DESC JitDesc[] =
{
	GB_DECLARE_STATIC("__Jit"),

	GB_STATIC_METHOD("Translate", "s", Jit_Translate, "(Class)s(From)s"),

	GB_END_DECLARE
};


GB_DESC *GB_CLASSES [] EXPORT =
{
	JitDesc,
	NULL
};


int EXPORT GB_INIT(void)
{
	return 0;
}


void EXPORT GB_EXIT()
{
}

