/***************************************************************************

  speed.c

  (c) 2019 Benoît Minisini <g4mba5@gmail.com>

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

#define __SPEED_C

#include "config.h"
#include "speed.h"

#if OS_LINUX

#if ARCH_PPC
	#include <asm-generic/ioctls.h>
	#include <asm-generic/termbits.h>
#else
	#include <asm/termios.h>
#endif

int ioctl(int fd, unsigned long request, ...);

int SetCustomSpeed(int fd, int speed)
{
	struct termios2 io;
	
	if (ioctl(fd, TCGETS2, &io))
		return 1;
	
	io.c_cflag &= ~CBAUD;
	io.c_cflag |= BOTHER;
	io.c_ispeed = speed;
	io.c_ospeed = speed;
	
	if (ioctl(fd, TCSETS2, &io))
		return 1;
	
	return 0;
}

#else

int SetCustomSpeed(int fd, int speed)
{
	return 1;
}

#endif

