/***************************************************************************

  SDLdebug.cpp

  (c) 2006-2008 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#include "SDLdebug.h"

#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

std::string DebugString = "";

void SDLdebug::Init()
{
	char* debug = std::getenv("DEBUG_GB_SDL");

	if (debug != NULL)
		DebugString = debug;
}

void SDLdebug::Print(const char* message, ...)
{
	std::string msg;
	const char *p;
	std::va_list list;
	va_start(list, message);

	if (DebugString.empty())
		return;

	for (p = message ; *p ; p++)
	{
		std::stringstream res;

		if (*p != '%')
		{
			msg.push_back(*p);
			continue;
		}
		
		p++;
		switch(*p)
		{
			int val;
			char* strval;

			//long
			case 'd' :
				val = va_arg(list, long);
				res << val;
				break;
			//hex
			case 'h' :
				val = va_arg(list, long);
				res << std::hex << val;
				break;
			//boolean
			case 'b' :
				val = va_arg(list, int);
				(val) ? res << "True": res << "False";
				break;
			//char string
			case 's' :
				strval = va_arg(list, char*);
				res << strval;
				break;
			default :
				res << "%" << *p;
		}
		msg = msg.append(res.str());
	}

	va_end(list);
	std::cerr << "==GB.SDL== " << msg << std::endl;
}
