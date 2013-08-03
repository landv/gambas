/***************************************************************************

  gbc_chown.c

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

#define __GBC_CHOWN_C

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gbc_chown.h"

void FILE_set_owner(const char *path, const char *template)
{
	struct stat info;
	
	if (geteuid())
		return;
	
	if (stat(template, &info))
		goto __ERROR;
	
	if (chown(path, info.st_uid, info.st_gid))
		goto __ERROR;
		
	return;
	
__ERROR:
	unlink(path);
	THROW("Cannot set file owner: &1: &2", path, strerror(errno));
}
