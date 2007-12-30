/***************************************************************************

  main.c

  Network Component

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

#define __MAIN_C

#include <stdio.h>

#include "CUdpSocket.h"
#include "CDnsClient.h"
#include "CSocket.h"
#include "CServerSocket.h"
#include "CSerialPort.h"
#include "CNet.h"
#include "gb_network.h"
#include "main.h"


#ifdef __cplusplus
extern "C" {
#endif

NETWORK_INTERFACE NET EXPORT;

GB_INTERFACE GB EXPORT;



GB_DESC *GB_CLASSES[] EXPORT =
{
  CDnsClientDesc,
  CSocketDesc,
  CServerSocketDesc,
  CUdpSocketDesc,
  CSerialPortDesc,
  CNetDesc,
  NULL
};



int EXPORT GB_INIT(void)
{
	NET.Socket.ConnectLocal=CSocket_connect_unix;
	NET.Socket.ConnectTCP=CSocket_connect_socket;
	NET.Socket.Peek=CSocket_peek_data;

	return 0;
}



void EXPORT GB_EXIT()
{

}


#ifdef _cpluscplus
}
#endif

