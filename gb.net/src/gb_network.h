/***************************************************************************

  gb_network.h

  Network component

  (c) 2003-2004 Daniel Campos Fernández <danielcampos@netcourrier.com>

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
	int (*ConnectLocal)	(CSOCKET *mythis,char *sPath,int lenpath);
	int (*ConnectTCP)	(CSOCKET *mythis,char *sHost,int lenhost,int myport);
	int (*Peek)		(CSOCKET *mythis,char **buf,int MaxLen);
	
} SOCKET_INTERFACE;

typedef struct
{
	SOCKET_INTERFACE Socket;
	
	
} NETWORK_INTERFACE;
