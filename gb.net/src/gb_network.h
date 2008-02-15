/***************************************************************************

  gb_network.h

  Network component

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

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
#include "CSocket.h"

typedef struct
{
	int (*ConnectLocal)	(void *_object,char *sPath,int lenpath);
	int (*ConnectTCP)	(void *_object,char *sHost,int lenhost,int myport);
	int (*Peek)		(void *_object,char **buf,int MaxLen);
	
} SOCKET_INTERFACE;

typedef struct
{
	SOCKET_INTERFACE Socket;
	
	
} NETWORK_INTERFACE;

